#include "ns3/al-fec-codec-openfec-rs.h"
#include "ns3/al-fec-header.h"
#include "ns3/core-module.h"
#include "ns3/type-id.h"

#include <optional>
#include <cmath>
#include <algorithm>
#include <arpa/inet.h>

extern "C" {
#include "openfec/lib_common/of_openfec_api.h"
}

#define VERBOSITY 0

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("AlFecCodecOpenfecRs");
NS_OBJECT_ENSURE_REGISTERED (AlFecCodecOpenfecRs);

AlFecCodecOpenfecRs::AlFecCodecOpenfecRs ()
    : m_session (nullptr),
      m_param ({0, 0}),
      m_sourcePacket (Ptr<Packet> ()),
      m_esi (0),
      m_encodedSymbol (nullptr),
      m_sourceSymbol (nullptr)
{
  NS_LOG_FUNCTION (this);
  m_param.encoding_symbol_length = m_symbolSize;
  m_param.m = m_rsM;
}

AlFecCodecOpenfecRs::~AlFecCodecOpenfecRs ()
{
  NS_LOG_FUNCTION (this);
  if (m_session)
    {
      of_release_codec_instance (m_session);
    }
  if (m_encodedSymbol)
    {

      unsigned int n = m_param.nb_repair_symbols + m_param.nb_source_symbols;
      NS_ASSERT (n > 0);
      for (unsigned int i = 0; i < n; i++)
        {
          free (m_encodedSymbol[i]);
        }
      free (m_encodedSymbol);
    }
  if (m_sourceSymbol)
    {
      NS_ASSERT (m_param.nb_source_symbols > 0);
      for (unsigned int i = 0; i < m_param.nb_source_symbols; i++)
        {
          free (m_sourceSymbol[i]);
        }
      free (m_sourceSymbol);
    }
}

TypeId
AlFecCodecOpenfecRs::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::AlFecCodecOpenfecRs")
          .SetParent<Object> ()
          .AddConstructor<AlFecCodecOpenfecRs> ()
          .AddAttribute ("m", "Reed-Solomon over GF(2^m)", UintegerValue (8),
                         MakeUintegerAccessor (&AlFecCodecOpenfecRs::m_rsM),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("symbolSize", "The symbol size in bytes", UintegerValue (8),
                         MakeUintegerAccessor (&AlFecCodecOpenfecRs::m_symbolSize),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("codeRate", "k/n", DoubleValue (1.0),
                         MakeDoubleAccessor (&AlFecCodecOpenfecRs::m_codeRate),
                         MakeDoubleChecker<double> (0.1, 1.0));
  return tid;
}

void
AlFecCodecOpenfecRs::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void
AlFecCodecOpenfecRs::SetSourceBlock (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  size_t payloadSize = p->GetSize ();
  size_t packetSize;
  size_t totalSymbol;
  NS_LOG_INFO ("Got new source block with size=" << payloadSize);
  NS_ASSERT_MSG (!m_session, "The codec has been initialized");

  // Serialize packet
  AlFecHeader::PayloadHeader payloadHeader;
  Ptr<Packet> encodingPacket;
  m_sourcePacket = p->Copy ();
  encodingPacket = m_sourcePacket->Copy ();

  // Payload header should be append to the encoding content
  // but not append to the metadata of original packet
  payloadHeader.SetSize (m_sourcePacket->GetSize ());
  encodingPacket->AddHeader (payloadHeader);
  packetSize = encodingPacket->GetSize ();

  // Calculate the encoding parameter
  m_param.nb_source_symbols = (unsigned int) ceil ((double) packetSize / m_symbolSize);
  totalSymbol = ceil (m_param.nb_source_symbols / m_codeRate);
  m_param.nb_repair_symbols = totalSymbol - m_param.nb_source_symbols;
  m_param.encoding_symbol_length = m_symbolSize;
  m_param.m = m_rsM;

  // Instance the encoder
  int ret;
  ret = of_create_codec_instance (&m_session, m_codecId, OF_ENCODER, VERBOSITY);
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Create encoder instance failed");
  ret = of_set_fec_parameters (m_session, reinterpret_cast<of_parameters_t *> (&m_param));
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Set FEC parameter failed");

  // Fill the source symbol
  m_encodedSymbol = reinterpret_cast<uint8_t **> (calloc (totalSymbol, sizeof (uint8_t *)));
  for (unsigned int esi = 0; esi < m_param.nb_source_symbols; esi++)
    {
      Ptr<Packet> fragment;
      unsigned int copyLen =
          std::min ((size_t) (esi + 1) * m_symbolSize, packetSize) - esi * m_symbolSize;
      m_encodedSymbol[esi] = reinterpret_cast<uint8_t *> (calloc (m_symbolSize, sizeof (uint8_t)));
      fragment = encodingPacket->CreateFragment (esi * m_symbolSize, copyLen);
      fragment->CopyData (m_encodedSymbol[esi], copyLen);
    }

  // Generate the repair symbol
  for (unsigned int esi = m_param.nb_source_symbols; esi < totalSymbol; esi++)
    {
      m_encodedSymbol[esi] = reinterpret_cast<uint8_t *> (calloc (m_symbolSize, sizeof (uint8_t)));
      ret = of_build_repair_symbol (m_session, reinterpret_cast<void **> (m_encodedSymbol), esi);
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Build repair symbol failed");
    }

  m_esi = 0;
}

std::optional<Ptr<Packet>>
AlFecCodecOpenfecRs::NextEncodedSymbol ()
{
  NS_LOG_FUNCTION (this << " " << m_esi);

  if (m_esi >= m_param.nb_repair_symbols + m_param.nb_source_symbols)
    {
      return std::nullopt;
    }
  uint8_t *payload = m_encodedSymbol[m_esi];
  // Ptr<Packet> p = Create<Packet> (payload, m_symbolSize);

  Ptr<Packet> p;
  AlFecHeader::EncodeHeader encodeHeader;

  // Replicate the source packet to preserve metadata and tags.
  p = m_sourcePacket->Copy ();
  // This method is a patch to Packet class.
  // It will override the content buffer of the packet without affecting metadata and tags.
  p->OverrideContent (payload, m_symbolSize);

  encodeHeader.SetEncodedSymbolId (m_esi);
  encodeHeader.SetK (m_param.nb_source_symbols);

  p->AddHeader (encodeHeader);

  m_esi++;

  return p;
}

std::optional<Ptr<Packet>>
AlFecCodecOpenfecRs::Decode (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  int ret;
  uint8_t *buf;
  AlFecHeader::EncodeHeader encodeHeader;
  p->RemoveHeader (encodeHeader);

  NS_LOG_INFO ("Decode with symbol " << encodeHeader);

  // The source packet has already decoded, there's no need to decode again.
  if (m_sourcePacket)
    {
      return m_sourcePacket;
    }

  // Instance the decoder session
  if (!m_session)
    {
      int k = encodeHeader.GetK ();
      int totalSymbol = ceil (k / m_codeRate);
      ret = of_create_codec_instance (&m_session, m_codecId, OF_DECODER, VERBOSITY);
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Create decoder instance failed");

      m_param.nb_source_symbols = k;
      m_param.nb_repair_symbols = totalSymbol - k;
      m_param.encoding_symbol_length = m_symbolSize;
      m_param.m = m_rsM;
      ret = of_set_fec_parameters (m_session, reinterpret_cast<of_parameters_t *> (&m_param));
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Set FEC parameter failed");
    }

  // Decode with new symbol
  NS_ASSERT_MSG (m_param.nb_source_symbols == encodeHeader.GetK (), "Mismatch source symbols");
  buf = reinterpret_cast<uint8_t *> (calloc (p->GetSize (), sizeof (uint8_t)));
  p->CopyData (buf, p->GetSize ());
  ret = of_decode_with_new_symbol (m_session, buf, encodeHeader.GetEncodedSymbolId ());
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Decode failed");

  if (!of_is_decoding_complete (m_session))
    {
      return std::nullopt;
    }

  // Retrieve source block
  m_sourceSymbol =
      reinterpret_cast<uint8_t **> (calloc (m_param.nb_source_symbols, sizeof (uint8_t *)));
  ret = of_get_source_symbols_tab (m_session, reinterpret_cast<void **> (m_sourceSymbol));
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Get source symbol failed");

  // Construct original packet
  size_t decodedContentLength = m_param.nb_source_symbols * m_symbolSize;
  uint8_t *decodedContent = reinterpret_cast<uint8_t *> (malloc (decodedContentLength));
  for (unsigned int esi = 0; esi < m_param.nb_source_symbols; esi++)
    {
      memcpy (decodedContent + (esi * m_symbolSize), m_sourceSymbol[esi], m_symbolSize);
    }
  AlFecHeader::PayloadHeader payloadHeader;
  Packet tmpPacket (decodedContent, decodedContentLength);
  tmpPacket.RemoveHeader (payloadHeader);
  decodedContent += payloadHeader.GetSerializedSize ();
  NS_LOG_INFO ("Original packet size=" << payloadHeader.GetSize ());

  // Replicate the received packet to preserve metadata and tags.
  m_sourcePacket = p->Copy ();
  // This method is a patch to Packet class.
  // It will override the content buffer of the packet without affecting metadata and tags.
  m_sourcePacket->OverrideContent (decodedContent, payloadHeader.GetSize ());

  return m_sourcePacket;
}

} // namespace ns3
