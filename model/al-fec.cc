#include "ns3/al-fec.h"
#include "ns3/al-fec-header.h"
#include "ns3/core-module.h"
#include "ns3/type-id.h"

#include "al-fec-info-tag.h"
#include "util.h"

#include <optional>
#include <cmath>
#include <algorithm>
#include <arpa/inet.h>

#include <iomanip>

#define ALIGN(x, n) (((x) + ((n) -1)) & (~((n) -1)))

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("AlFec");
NS_OBJECT_ENSURE_REGISTERED (AlFec);

AlFec::AlFec () : m_originalPacket (Ptr<Packet> ()), m_codec (nullptr)
{
  NS_LOG_FUNCTION (this);
}

AlFec::AlFec (AlFecCodec *codec) : m_originalPacket (Ptr<Packet> ()), m_codec (codec)
{
  NS_LOG_FUNCTION (this);
}

AlFec::~AlFec ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
AlFec::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AlFec").AddConstructor<AlFec> ().SetParent<Object> ();
  // .AddAttribute ("codec", "Pointer to the codec implementation", PointerValue (),
  //                MakePointerAccessor (&AlFec::m_codec), MakePointerChecker<AlFecCodec> ());
  return tid;
}

void
AlFec::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void
AlFec::SetCodec (AlFecCodec *codec)
{
  m_codec = codec;
}

size_t
AlFec::EncodePacket (Ptr<Packet> originalPacket)
{
  NS_LOG_FUNCTION (this);

  const size_t payloadSize = originalPacket->GetSize ();
  NS_LOG_INFO ("Got new packet with size=" << payloadSize);
  NS_ASSERT_MSG (m_codec != nullptr, "The codec hasn't been initialized");

  // Serialize packet
  AlFecHeader::PayloadHeader payloadHeader;
  Ptr<Packet> encodingPacket;
  m_originalPacket = originalPacket;
  encodingPacket = m_originalPacket->Copy ();

  // Padding
  size_t symbolSize = m_codec->GetSymbolSize ();
  size_t packetSize = m_originalPacket->GetSize () + payloadHeader.GetSerializedSize ();
  size_t paddingSize = (symbolSize - (packetSize % symbolSize)) % symbolSize;
  encodingPacket->AddPaddingAtEnd (paddingSize);

  // Payload header should be append to the encoding content
  payloadHeader.SetPaddingSize (paddingSize);
  encodingPacket->AddHeader (payloadHeader);

  const size_t serializedSize = encodingPacket->GetSerializedSize ();

  uint8_t *serializeBuf = new uint8_t[serializedSize];
  const uint8_t *p = serializeBuf;
  encodingPacket->Serialize (serializeBuf, serializedSize);

  NS_LOG_INFO ("Source packet: " << *encodingPacket << "\n"
                               << printBuffer (serializeBuf, serializedSize));

  // Retrieve the context and content through serialization
  uint32_t nixSize, byteTagSize, packetTagSize, metaSize, bufSize, contextSize;
  nixSize = *reinterpret_cast<const uint32_t *> (p);
  p += ALIGN (nixSize, sizeof (uint32_t));
  byteTagSize = *reinterpret_cast<const uint32_t *> (p);
  p += ALIGN (byteTagSize, sizeof (uint32_t));
  packetTagSize = *reinterpret_cast<const uint32_t *> (p);
  p += ALIGN (packetTagSize, sizeof (uint32_t));
  metaSize = *reinterpret_cast<const uint32_t *> (p);
  p += ALIGN (metaSize, sizeof (uint32_t));
  bufSize = *reinterpret_cast<const uint32_t *> (p);
  contextSize = nixSize + byteTagSize + packetTagSize + metaSize;
  p += sizeof (uint32_t);

  NS_LOG_INFO ("Serialized size: Context=" << contextSize << ", Buffer=" << bufSize);

  m_sourceContext.AddAtStart (contextSize);
  m_sourceContext.Begin ().Write (serializeBuf, contextSize);
  m_sourceBlock.Deserialize (p, bufSize);

  delete[] serializeBuf;

  NS_LOG_INFO ("Buffer size=" << m_sourceBlock.GetSize ());

  m_codec->SetSourceBlock (m_sourceBlock);

  return m_codec->GetN ();
}

std::optional<Ptr<Packet>>
AlFec::NextEncodedPacket ()
{
  NS_LOG_FUNCTION (this);

  std::optional<std::pair<unsigned int, Buffer>> encodedBlock;
  unsigned int esi;
  Buffer content;
  Ptr<Packet> p;
  AlFecHeader::EncodeHeader encodeHeader;
  uint8_t *buf;

  encodedBlock = m_codec->NextEncodedBlock ();
  if (!encodedBlock)
    {
      return std::nullopt;
    }
  esi = encodedBlock->first;
  content = encodedBlock->second;

  buf = new uint8_t[content.GetSize ()];
  content.CopyData (buf, content.GetSize ());
  p = Create<Packet> (buf, content.GetSize ());
  delete[] buf;

  encodeHeader.SetEncodedSymbolId (esi);
  p->AddHeader (encodeHeader);

  // Append K and the context of original packet
  AlFecInfoTag encodeTag;
  encodeTag.SetPacketContext (m_sourceContext);
  encodeTag.SetK (m_codec->GetK ());
  encodeTag.SetSymbolSize (m_codec->GetSymbolSize ());
  p->AddByteTag (encodeTag);

  NS_LOG_LOGIC ("New encoded block " << encodeHeader << "; " << encodeTag);

  return p;
}

std::optional<Ptr<Packet>>
AlFec::DecodePacket (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  // int ret;
  // uint8_t *buf;
  AlFecHeader::EncodeHeader encodeHeader;
  AlFecInfoTag encodeTag;
  p->RemoveHeader (encodeHeader);
  p->FindFirstMatchingByteTag (encodeTag);

  NS_LOG_LOGIC ("Decode with block " << encodeHeader << "; " << encodeTag);

  // The source packet has already decoded, there's no need to decode again.
  if (m_originalPacket)
    {
      return m_originalPacket;
    }

  NS_ASSERT_MSG (m_codec != nullptr, "The codec hasn't been initialized");

  // Initialize the decoder
  if (m_codec->GetK () == 0)
    {
      uint16_t k = encodeTag.GetK ();
      m_codec->SetK (k);
    }

  // Decode with new symbol
  uint16_t symbolSize = encodeTag.GetSymbolSize ();
  uint8_t *buf = new uint8_t[symbolSize];
  Buffer newBlock;
  std::optional<Buffer> decodedBlock;
  p->CopyData (buf, symbolSize);
  newBlock.AddAtStart (symbolSize);
  newBlock.Begin ().Write (buf, symbolSize);
  decodedBlock = m_codec->Decode (newBlock, encodeHeader.GetEncodedSymbolId ());
  delete[] buf;

  if (!decodedBlock)
    {
      return std::nullopt;
    }

  NS_LOG_INFO ("Length of decoded block=" << decodedBlock->GetSize ());

  // Remove the padding of decoded packet at the buffer level
  Buffer header = *decodedBlock;
  AlFecHeader::PayloadHeader payloadHeader;
  payloadHeader.Deserialize (header.Begin ());
  size_t paddingSize = payloadHeader.GetPaddingSize ();
  NS_LOG_INFO ("Padding size=" << paddingSize);
  decodedBlock->RemoveAtEnd (paddingSize);
  NS_LOG_INFO ("Buffer size=" << decodedBlock->GetSize ());

  // Restore the context of original packet
  uint8_t *cur;
  Buffer packetContext = encodeTag.GetPacketContext ();
  size_t serializedSize =
      packetContext.GetSize () + (sizeof (uint32_t) + decodedBlock->GetSerializedSize ());
  serializedSize = ALIGN (serializedSize, sizeof (uint32_t));
  buf = new uint8_t[serializedSize];
  cur = buf;
  packetContext.CopyData (cur, packetContext.GetSize ());
  cur += ALIGN (packetContext.GetSize (), sizeof (uint32_t));
  *reinterpret_cast<uint32_t *> (cur) = decodedBlock->GetSerializedSize () + sizeof (uint32_t);
  cur += sizeof (uint32_t);
  decodedBlock->Serialize (cur, decodedBlock->GetSerializedSize ());

  // Deserialized the decoded packet with original context
  Ptr<Packet> decodedPacket = Create<Packet> (buf, serializedSize, true);

  // Process the deserialized packet

  decodedPacket->RemoveHeader (payloadHeader);

  m_originalPacket = decodedPacket;

  return m_originalPacket;
}

} // namespace ns3
