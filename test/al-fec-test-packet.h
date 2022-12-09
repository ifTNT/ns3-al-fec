/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#ifndef TEST_AL_FEC_PACKET_H
#define TEST_AL_FEC_PACKET_H

#include "ns3/test.h"

using namespace ns3;

class AlFecPacketTestSuite : public TestSuite
{
public:
  AlFecPacketTestSuite ();
};

/**
 * Test 1. Successfully encapsulation
 */
class EncapsulateTestCase : public TestCase
{
public:
  EncapsulateTestCase ();
  virtual ~EncapsulateTestCase ();
  const int symbolSize = 16;
  const double codeRate = 0.5;
  const int payloadSize = 1000;

private:
  virtual void DoRun (void);
  ObjectFactory m_codecFactory;
};

/**
 * Test 2. Successfully interpretation
 */
class InterpretationTestCase : public TestCase
{
public:
  InterpretationTestCase ();
  virtual ~InterpretationTestCase ();
  const int symbolSize = 16;
  const double codeRate = 0.5;
  const int payloadSize = 1000;

private:
  virtual void DoRun (void);
  ObjectFactory m_codecFactory;
};

#endif /* TEST_AL_FEC_PACKET_H */
