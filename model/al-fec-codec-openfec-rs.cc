#include "ns3/al-fec-codec-openfec-rs.h"
#include "ns3/core-module.h"
#include "ns3/type-id.h"

#include <optional>
#include <cmath>
#include <algorithm>

extern "C" {
#include "openfec/lib_common/of_openfec_api.h"
}

#define VERBOSITY 2

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("AlFecCodecOpenfecRs");
NS_OBJECT_ENSURE_REGISTERED (AlFecCodecOpenfecRs);

AlFecCodecOpenfecRs::AlFecCodecOpenfecRs ()
    : m_session (nullptr),
      m_param ({0, 0}),
      m_sourceBlock (std::nullopt),
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
          .AddAttribute ("symbolSize", "The symbol size in bytes", UintegerValue (16),
                         MakeUintegerAccessor (&AlFecCodecOpenfecRs::m_symbolSize),
                         MakeUintegerChecker<uint32_t> ())
          .AddAttribute ("codeRate", "k/n", DoubleValue (0.5),
                         MakeDoubleAccessor (&AlFecCodecOpenfecRs::m_codeRate),
                         MakeDoubleChecker<double> (0.1, 1.0));
  return tid;
}

void
AlFecCodecOpenfecRs::DoDispose ()
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

std::pair<size_t, size_t>
AlFecCodecOpenfecRs::SetSourceBlock (Buffer p)
{
  NS_LOG_FUNCTION (this);

  size_t sourceBlockSize = p.GetSize ();
  NS_LOG_INFO ("Got new source block with size=" << sourceBlockSize);
  NS_ASSERT_MSG (!m_session, "The codec has been initialized");

  // Calculate the encoding parameter
  SetK (static_cast<size_t> (ceil (static_cast<double> (sourceBlockSize) / m_symbolSize)));
  SetN (ceil (static_cast<double> (m_k) / m_codeRate));
  m_param.nb_source_symbols = m_k;
  m_param.nb_repair_symbols = m_n - m_k;
  m_param.encoding_symbol_length = m_symbolSize;
  m_param.m = m_rsM;

  // Instance the encoder
  int ret;
  ret = of_create_codec_instance (&m_session, m_codecId, OF_ENCODER, VERBOSITY);
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Create encoder instance failed");
  ret = of_set_fec_parameters (m_session, reinterpret_cast<of_parameters_t *> (&m_param));
  NS_ASSERT_MSG (ret == OF_STATUS_OK, "Set FEC parameter failed");

  // Fill the source symbol
  m_encodedSymbol = reinterpret_cast<uint8_t **> (calloc (m_n, sizeof (uint8_t *)));
  for (unsigned int esi = 0; esi < m_param.nb_source_symbols; esi++)
    {
      unsigned int copyLen =
          std::min ((size_t) (esi + 1) * m_symbolSize, sourceBlockSize) - esi * m_symbolSize;
      Buffer fragment = p.CreateFragment (esi * m_symbolSize, copyLen);
      m_encodedSymbol[esi] = reinterpret_cast<uint8_t *> (calloc (m_symbolSize, sizeof (uint8_t)));
      fragment.CopyData (m_encodedSymbol[esi], copyLen);
    }

  // Generate the repair symbol
  for (unsigned int esi = m_param.nb_source_symbols; esi < m_n; esi++)
    {
      m_encodedSymbol[esi] = reinterpret_cast<uint8_t *> (calloc (m_symbolSize, sizeof (uint8_t)));
      ret = of_build_repair_symbol (m_session, reinterpret_cast<void **> (m_encodedSymbol), esi);
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Build repair symbol failed");
    }

  // Reset the internal state
  m_esi = 0;

  return std::make_pair (m_n, m_k);
}

std::optional<std::pair<unsigned int, Buffer>>
AlFecCodecOpenfecRs::NextEncodedBlock ()
{
  NS_LOG_FUNCTION (this << " " << m_esi);

  if (m_esi >= m_param.nb_repair_symbols + m_param.nb_source_symbols)
    {
      return std::nullopt;
    }
  uint8_t *payload = m_encodedSymbol[m_esi];
  Buffer newBlock;
  newBlock.AddAtStart (m_symbolSize);
  newBlock.Begin ().Write (payload, m_symbolSize);

  return std::make_pair (m_esi++, newBlock);
}

std::optional<Buffer>
AlFecCodecOpenfecRs::Decode (Buffer p, unsigned int esi)
{
  NS_LOG_FUNCTION (this);

  int ret;
  uint8_t *buf;

  NS_LOG_LOGIC ("Decode with block esi=" << esi);

  // The source packet has already decoded, there's no need to decode again.
  if (m_sourceBlock)
    {
      return *m_sourceBlock;
    }

  // Instance the decoder session
  if (!m_session)
    {
      NS_ASSERT_MSG (m_k > 0, "K is not initialize");
      SetN (m_k / m_codeRate);
      ret = of_create_codec_instance (&m_session, m_codecId, OF_DECODER, VERBOSITY);
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Create decoder instance failed");

      m_param.nb_source_symbols = m_k;
      m_param.nb_repair_symbols = m_n - m_k;
      m_param.encoding_symbol_length = m_symbolSize;
      m_param.m = m_rsM;
      ret = of_set_fec_parameters (m_session, reinterpret_cast<of_parameters_t *> (&m_param));
      NS_ASSERT_MSG (ret == OF_STATUS_OK, "Set FEC parameter failed");
    }

  // Decode with new symbol
  buf = reinterpret_cast<uint8_t *> (calloc (p.GetSize (), sizeof (uint8_t)));
  p.CopyData (buf, p.GetSize ());
  ret = of_decode_with_new_symbol (m_session, buf, esi);
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
  for (unsigned int i = 0; i < m_param.nb_source_symbols; i++)
    {
      memcpy (decodedContent + (i * m_symbolSize), m_sourceSymbol[i], m_symbolSize);
    }

  Buffer sourceBlock;
  sourceBlock.AddAtStart (decodedContentLength);
  sourceBlock.Begin ().Write (decodedContent, decodedContentLength);
  m_sourceBlock = std::make_optional<Buffer> (sourceBlock);

  NS_LOG_INFO ("Successfully decode source block. size=" << sourceBlock.GetSize ()
                                                         << " (may contain padding)");
  free (decodedContent);
  return sourceBlock;
}

} // namespace ns3
