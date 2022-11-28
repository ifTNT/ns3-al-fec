/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/packet.h"

#include "al-fec-test-codec-openfec-rs.h"
#include "ns3/al-fec-codec-openfec-rs.h"
#include "ns3/al-fec-header.h"

#include <optional>
#include <cmath>
#include <random>
#include <unistd.h>
#include <fcntl.h>

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

AlFecCodecOpenfecRsTestSuite::AlFecCodecOpenfecRsTestSuite () : TestSuite ("al-fec-codec-openfec-rs", SYSTEM)
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
  Ptr<AlFecCodecOpenfecRs> encoder = m_codecFactory.Create<AlFecCodecOpenfecRs> ();

  uint8_t *buf = (uint8_t *) malloc (packetSize * sizeof (uint8_t));
  std::optional<Packet> encodedPacket;
  Packet p;
  AlFecHeader header;
  int k = ceil ((double) packetSize / symbolSize);
  int cntSymbol = 0;
  fillRandomBytes (buf, packetSize);
  p = Packet (buf, packetSize);

  printBuffer (buf, packetSize);

  encoder->SetSourceBlock (p);
  encodedPacket = encoder->NextEncodedSymbol ();
  while (encodedPacket)
    {
      cntSymbol++;
      encodedPacket->RemoveHeader (header);
      NS_TEST_ASSERT_MSG_EQ (header.GetK (), k, "Calculated source symbols mismatch");
      NS_TEST_ASSERT_MSG_EQ (encodedPacket->GetSize (), symbolSize, "Symbol size mismatch");
      encodedPacket = encoder->NextEncodedSymbol ();
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
  Ptr<AlFecCodecOpenfecRs> encoder = m_codecFactory.Create<AlFecCodecOpenfecRs> ();
  Ptr<AlFecCodecOpenfecRs> decoder = m_codecFactory.Create<AlFecCodecOpenfecRs> ();

  uint8_t *buf = (uint8_t *) malloc (packetSize * sizeof (uint8_t));
  std::optional<Packet> encodedPacket;
  std::optional<Packet> decodedPacket;
  std::vector<Packet> packetList;
  Packet p;
  std::random_device rd;
  std::mt19937 gen (rd ());

  fillRandomBytes (buf, packetSize);
  p = Packet (buf, packetSize);

  encoder->SetSourceBlock (p);
  encodedPacket = encoder->NextEncodedSymbol ();
  while (encodedPacket)
    {
      packetList.push_back (*encodedPacket);
      encodedPacket = encoder->NextEncodedSymbol ();
    }

  shuffle (packetList.begin (), packetList.end (), gen);

  int i;
  int k = ceil ((double) packetSize / symbolSize);
  for (i = 0; i < (int) packetList.size (); i++)
    {
      decodedPacket = decoder->Decode (packetList[i]);
      if (decodedPacket)
        {
          break;
        }
    }

  NS_TEST_ASSERT_MSG_EQ (i+1, k, "Should decode with k symbols");

  // Check if we can decode with more symbol
  for (i = k; i < (int) packetList.size (); i++)
    {
      decodedPacket = decoder->Decode (packetList[i]);
    }

  uint8_t *rx_buf = (uint8_t *) malloc (decodedPacket->GetSize ());
  decodedPacket->CopyData (rx_buf, decodedPacket->GetSize ());
  std::cout << "Original packet" << std::endl;
  printBuffer (buf, packetSize);
  std::cout << "Decoded packet" << std::endl;
  printBuffer (rx_buf, decodedPacket->GetSize ());

  NS_TEST_ASSERT_MSG_EQ (decodedPacket->GetSize (), p.GetSize (),
                         "Length of decode packet mismatch");
  for (int i = 0; i < packetSize; i++)
    {
      NS_TEST_ASSERT_MSG_EQ (buf[i], rx_buf[i], "Decode content mismatch");
    }
}
