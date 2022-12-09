/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/packet.h"

#include "al-fec-test-packet.h"
#include "ns3/al-fec.h"
#include "ns3/al-fec-codec-openfec-rs.h"
#include "ns3/al-fec-header.h"

#include "ns3/icmpv4.h"

#include <optional>
#include <cmath>
#include <random>
#include <unistd.h>
#include <fcntl.h>
#include <limits>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("AlFecPacketTest");

void fillRandomBytes (uint8_t *buf, unsigned int size);

void printBuffer (uint8_t *buf, unsigned int size);

class DummyCodec : public Object, public AlFecCodec
{
public:
  DummyCodec ()
  {
    SetSymbolSize (16);
  }
  ~DummyCodec ()
  {
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId
  GetTypeId (void)
  {
    return TypeId ("ns3::DummyCodec").SetParent<Object> ();
  }
  virtual void
  DoDispose ()
  {
  }

  /**
   * \brief Specify the source block
   * 
   * \return {The number of encoded block (n), the number of source block (k)}
  */
  std::pair<size_t, size_t>
  SetSourceBlock (Buffer p)
  {
    m_sourceBlock = p;

    // Padding
    m_sourceBlock.AddAtEnd ((m_symbolSize - (p.GetSize () % m_symbolSize)) % m_symbolSize);

    SetK (ceil (static_cast<double> (m_sourceBlock.GetSize ()) / m_symbolSize));
    SetN (m_k);
    m_esi = 0;
    return std::make_pair (m_n, m_k);
  }

  /**
   * \brief Get the next encoded symbol
   * 
   * \return Return the next unsent encoded block.
   * If there's no unsent encoded block, return std::nullopt
  */
  std::optional<std::pair<unsigned int, Buffer>>
  NextEncodedBlock ()
  {
    if (m_sourceBlock.GetSize () == 0)
      {
        return std::nullopt;
      }
    Buffer newBlock = m_sourceBlock.CreateFragment (0, m_symbolSize);
    m_sourceBlock.RemoveAtStart (m_symbolSize);
    return std::make_pair (m_esi++, newBlock);
  }

  /**
   * \brief Decode source block with received block
   * 
   * \param p The content of received block
   * \param esi The received Encoded Symbol ID
   * 
   * \return If the source block successfully decoded, return the decoded block.
   * Other, return std::nullopt
  */
  std::optional<Buffer>
  Decode (Buffer p, unsigned int esi)
  {
    m_sourceBlock.AddAtEnd (p.GetSize ());
    m_sourceBlock.End ().Write (p.Begin (), p.End ());
    return std::nullopt;
  }

private:
  Buffer m_sourceBlock;
  unsigned int m_esi = 0;
};

/**
 * TestSuite
 */

AlFecPacketTestSuite::AlFecPacketTestSuite () : TestSuite ("al-fec-packet", SYSTEM)
{
  LogLevel logLevel = (LogLevel) (LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

  LogComponentEnable ("AlFecPacketTest", logLevel);
  AddTestCase (new EncapsulateTestCase (), TestCase::QUICK);
  AddTestCase (new InterpretationTestCase (), TestCase::QUICK);
}

static AlFecPacketTestSuite packetTestSuite;

/**
 * TestCase 1
 */

EncapsulateTestCase::EncapsulateTestCase () : TestCase ("Check encapsulation")
{
  NS_LOG_INFO ("Creating EncapsulateTestCase");
  m_codecFactory.SetTypeId ("ns3::AlFecCodecOpenfecRs");
  m_codecFactory.Set ("symbolSize", UintegerValue (symbolSize));
  m_codecFactory.Set ("codeRate", DoubleValue (codeRate));
}

EncapsulateTestCase::~EncapsulateTestCase ()
{
}

void
EncapsulateTestCase::DoRun (void)
{
  // Ptr<DummyCodec> encoderObj = CreateObject<DummyCodec> ();
  Ptr<AlFecCodecOpenfecRs> encoderObj = m_codecFactory.Create<AlFecCodecOpenfecRs> ();
  AlFecCodec *codec = static_cast<AlFecCodec *> (GetPointer (encoderObj));
  Ptr<AlFec> encoder = CreateObject<AlFec> ();
  encoder->SetCodec (codec);

  uint8_t *buf = reinterpret_cast<uint8_t *> (malloc (payloadSize));
  std::optional<Ptr<Packet>> encodedPacket;
  Ptr<Packet> p;
  size_t n;
  size_t cntPacket = 0;
  fillRandomBytes (buf, payloadSize);

  p = Create<Packet> (buf, payloadSize);

  n = encoder->EncodePacket (p);
  while (encodedPacket = encoder->NextEncodedPacket ())
    {
      cntPacket++;
      (*encodedPacket)->CopyData (buf, (*encodedPacket)->GetSize ());
      NS_LOG_INFO (**encodedPacket);
      (*encodedPacket)->PrintPacketTags (std::cout);
      printBuffer (buf, (*encodedPacket)->GetSize ());
    }
  NS_TEST_ASSERT_MSG_EQ (cntPacket, n, "Total symbols mismatch");
  free (buf);
  encoderObj->Dispose ();
}

/**
 * TestCase 2
 */

InterpretationTestCase::InterpretationTestCase () : TestCase ("Check interpretation")
{
  NS_LOG_INFO ("Creating InterpretationTestCase");
  m_codecFactory.SetTypeId ("ns3::AlFecCodecOpenfecRs");
  m_codecFactory.Set ("symbolSize", UintegerValue (symbolSize));
  m_codecFactory.Set ("codeRate", DoubleValue (codeRate));
}

InterpretationTestCase::~InterpretationTestCase ()
{
}

void
InterpretationTestCase::DoRun (void)
{
  // Ptr<DummyCodec> encoderObj = CreateObject<DummyCodec> ();
  Ptr<AlFecCodecOpenfecRs> encoderObj = m_codecFactory.Create<AlFecCodecOpenfecRs> ();
  AlFecCodec *encodeCodec = static_cast<AlFecCodec *> (GetPointer (encoderObj));
  Ptr<AlFec> encoder = CreateObject<AlFec> ();
  encoder->SetCodec (encodeCodec);

  uint8_t *buf = new uint8_t[payloadSize];
  std::optional<Ptr<Packet>> encodedPacket;
  Ptr<Packet> originalPacket;
  fillRandomBytes (buf, payloadSize);

  printBuffer (buf, payloadSize);

  originalPacket = Create<Packet> (buf, payloadSize);
  delete[] buf;

  Icmpv4Echo testHeader;
  std::random_device rd;
  std::mt19937 gen (rd ());
  std::uniform_int_distribution<uint16_t> uniDist (0, std::numeric_limits<uint16_t>::max ());
  testHeader.SetIdentifier (uniDist (gen));
  testHeader.SetSequenceNumber (uniDist (gen));
  originalPacket->AddHeader (testHeader);

  std::vector<Ptr<Packet>> packetList;

  encoder->EncodePacket (originalPacket);
  while (encodedPacket = encoder->NextEncodedPacket ())
    {
      packetList.push_back (*encodedPacket);
    }

  shuffle (packetList.begin (), packetList.end (), gen);
  Ptr<AlFecCodecOpenfecRs> decoderObj = m_codecFactory.Create<AlFecCodecOpenfecRs> ();
  AlFecCodec *decodeCodec = static_cast<AlFecCodec *> (GetPointer (decoderObj));
  Ptr<AlFec> decoder = CreateObject<AlFec> ();
  decoder->SetCodec (decodeCodec);

  std::optional<Ptr<Packet>> decodedPacket;
  for (auto rcvdPacket : packetList)
    {
      decodedPacket = decoder->DecodePacket (rcvdPacket);
    }

  Icmpv4Echo rcvdHeader;
  (*decodedPacket)->PeekHeader (rcvdHeader);

  // Compare the decoded packet with original packet
  NS_TEST_ASSERT_MSG_EQ (originalPacket->GetSerializedSize (),
                         (*decodedPacket)->GetSerializedSize (), "Serialized size mismatch");
  size_t serializedSize = originalPacket->GetSerializedSize ();
  uint8_t *txBuf = new uint8_t[serializedSize];
  originalPacket->Serialize (txBuf, serializedSize);
  uint8_t *rxBuf = new uint8_t[serializedSize];
  (*decodedPacket)->Serialize (rxBuf, serializedSize);

  NS_LOG_INFO ("Original packet: " << *originalPacket);
  printBuffer (txBuf, serializedSize);
  NS_LOG_INFO ("Decoded packet: " << **decodedPacket);
  printBuffer (rxBuf, serializedSize);
  NS_TEST_ASSERT_MSG_EQ (rcvdHeader.GetIdentifier (), testHeader.GetIdentifier (),
                         "Header mismatch");
  NS_TEST_ASSERT_MSG_EQ (rcvdHeader.GetSequenceNumber (), testHeader.GetSequenceNumber (),
                         "Header mismatch");
  for (size_t i = 0; i < static_cast<size_t> (serializedSize); i++)
    {
      NS_TEST_ASSERT_MSG_EQ (txBuf[i], rxBuf[i], "Decode content mismatch");
    }
  delete[] rxBuf;
  delete[] txBuf;
  encoderObj->Dispose ();
  decoderObj->Dispose ();
}
