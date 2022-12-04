/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/packet.h"

#include "al-fec-test-codec-openfec-rs.h"
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

NS_LOG_COMPONENT_DEFINE ("AlFecCodecOpenfecRsTest");

void
fillRandomBytes (uint8_t *buf, unsigned int size)
{
  int fd = open ("/dev/random", O_RDONLY);
  read (fd, buf, size);
}

void
printBuffer (uint8_t *buf, unsigned int size)
{
  for (unsigned int i = 0; i < size; i++)
    {
      printf ("%02x ", *(buf + i));
    }
  puts ("\n");
}

/**
 * TestSuite
 */

AlFecCodecOpenfecRsTestSuite::AlFecCodecOpenfecRsTestSuite ()
    : TestSuite ("al-fec-codec-openfec-rs", SYSTEM)
{
  LogLevel logLevel = (LogLevel) (LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

  LogComponentEnable ("AlFecCodecOpenfecRsTest", logLevel);
  AddTestCase (new OpenfecRsEncodeTestCase (), TestCase::QUICK);
  AddTestCase (new OpenfecRsDecodeTestCase (), TestCase::QUICK);
}

static AlFecCodecOpenfecRsTestSuite openfecRsTestSuite;

/**
 * TestCase 1
 */

OpenfecRsEncodeTestCase::OpenfecRsEncodeTestCase () : TestCase ("Check encoding")
{
  NS_LOG_INFO ("Creating OpenfecRsEncodeTestCase");
  m_codecFactory.SetTypeId ("ns3::AlFecCodecOpenfecRs");
  m_codecFactory.Set ("symbolSize", UintegerValue (symbolSize));
  m_codecFactory.Set ("codeRate", DoubleValue (codeRate));
}

OpenfecRsEncodeTestCase::~OpenfecRsEncodeTestCase ()
{
}

void
OpenfecRsEncodeTestCase::DoRun (void)
{
  AlFecCodec *encoder = GetPointer (m_codecFactory.Create<AlFecCodecOpenfecRs> ());

  uint8_t *buf = reinterpret_cast<uint8_t *>(malloc (payloadSize));
  std::optional<Ptr<Packet>> encodedPacket;
  Ptr<Packet> p;
  AlFecHeader::EncodeHeader header;
  int k;
  int cntSymbol = 0;
  fillRandomBytes (buf, payloadSize);
  p = Create<Packet> (buf, payloadSize);
  k = ceil ((double) p->GetSize () / symbolSize);

  printBuffer (buf, payloadSize);

  encoder->SetSourceBlock (p);
  while (encodedPacket = encoder->NextEncodedSymbol ())
    {
      cntSymbol++;
      (*encodedPacket)->RemoveHeader (header);
      NS_TEST_ASSERT_MSG_EQ (header.GetK (), k, "Calculated source symbols mismatch");
      NS_TEST_ASSERT_MSG_EQ ((*encodedPacket)->GetSize (), symbolSize, "Symbol size mismatch");
    }
  NS_TEST_ASSERT_MSG_EQ (cntSymbol, ceil (k / codeRate), "Total symbols mismatch");
}

/**
 * TestCase 2
 */

OpenfecRsDecodeTestCase::OpenfecRsDecodeTestCase () : TestCase ("Check decoding")
{
  NS_LOG_INFO ("Creating OpenfecRsDecodeTestCase");
  m_codecFactory.SetTypeId ("ns3::AlFecCodecOpenfecRs");
  m_codecFactory.Set ("symbolSize", UintegerValue (symbolSize));
  m_codecFactory.Set ("codeRate", DoubleValue (codeRate));
}

OpenfecRsDecodeTestCase::~OpenfecRsDecodeTestCase ()
{
}

void
OpenfecRsDecodeTestCase::DoRun (void)
{
  AlFecCodec *encoder = GetPointer (m_codecFactory.Create<AlFecCodecOpenfecRs> ());
  AlFecCodec *decoder = GetPointer (m_codecFactory.Create<AlFecCodecOpenfecRs> ());

  Ptr<Packet> p;
  Icmpv4Echo testHeader, rcvdHeader;
  size_t packetSize;
  uint8_t *buf = reinterpret_cast<uint8_t*> (malloc (payloadSize));
  std::optional<Ptr<Packet>> encodedPacket;
  std::optional<Ptr<Packet>> decodedPacket;
  std::vector<Ptr<Packet>> packetList;
  std::random_device rd;
  std::mt19937 gen (rd ());
  std::uniform_int_distribution<uint16_t> uniDist (0, std::numeric_limits<uint16_t>::max ());

  fillRandomBytes (buf, payloadSize);
  p = Create<Packet> (buf, payloadSize);
  testHeader.SetIdentifier (uniDist (gen));
  testHeader.SetSequenceNumber (uniDist (gen));
  p->AddHeader (testHeader);
  packetSize = p->GetSerializedSize ();
  buf = reinterpret_cast<uint8_t *>(realloc (buf, packetSize));
  p->Serialize (buf, packetSize);

  encoder->SetSourceBlock (p);
  encodedPacket = encoder->NextEncodedSymbol ();
  while (encodedPacket = encoder->NextEncodedSymbol ())
    {
      packetList.push_back (*encodedPacket);
      NS_LOG_INFO("Encoded packet: " << **encodedPacket);
    }

  shuffle (packetList.begin (), packetList.end (), gen);

  int i;
  int k = ceil ((double) p->GetSize () / symbolSize);
  for (i = 0; i < (int) packetList.size (); i++)
    {
      decodedPacket = decoder->Decode (packetList[i]);
      if (decodedPacket)
        {
          break;
        }
    }

  NS_TEST_ASSERT_MSG_EQ (i + 1, k, "Should decode with k symbols");

  // Check if we can decode with more symbol
  for (i = k; i < (int) packetList.size (); i++)
    {
      decodedPacket = decoder->Decode (packetList[i]);
    }

  size_t rcvdSize = (*decodedPacket)->GetSerializedSize ();
  uint8_t *rx_buf = reinterpret_cast<uint8_t *>(malloc (rcvdSize));
  (*decodedPacket)->Serialize (rx_buf, rcvdSize);
  std::cout << "Original packet serialization" << std::endl;
  printBuffer (buf, packetSize);
  std::cout << "Decoded packet serialization" << std::endl;
  printBuffer (rx_buf, rcvdSize);

  NS_TEST_ASSERT_MSG_EQ (rcvdSize, packetSize, "Length of decode packet mismatch");
  for (size_t i = 0; i < packetSize; i++)
    {
      NS_TEST_ASSERT_MSG_EQ (buf[i], rx_buf[i], "Decode content mismatch");
    }

  NS_LOG_INFO ("Original packet: " << *p);
  NS_LOG_INFO ("Decoded packet: " << **decodedPacket);
  
  (*decodedPacket)->RemoveHeader (rcvdHeader);
  NS_TEST_ASSERT_MSG_EQ (rcvdHeader.GetIdentifier (), testHeader.GetIdentifier (),
                         "Header mismatch");
  NS_TEST_ASSERT_MSG_EQ (rcvdHeader.GetSequenceNumber (), testHeader.GetSequenceNumber (),
                         "Header mismatch");
}
