/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#ifndef TEST_AL_FEC_CODEC_OPENFEC_RS_H
#define TEST_AL_FEC_CODEC_OPENFEC_RS_H

#include "ns3/test.h"

using namespace ns3;

class AlFecCodecOpenfecRsTestSuite : public TestSuite
{
public:
  AlFecCodecOpenfecRsTestSuite ();
};

/**
 * Test 1. Successfully encode
 */
class OpenfecRsEncodeTestCase : public TestCase
{
public:
  OpenfecRsEncodeTestCase ();
  virtual ~OpenfecRsEncodeTestCase ();
  const unsigned int symbolSize = 16;
  const double codeRate = 0.25;
  const int packetSize = 1000;

private:
  virtual void DoRun (void);
  ObjectFactory m_codecFactory;
};

/**
 * Test 2. Successfully decode
 */
class OpenfecRsDecodeTestCase : public TestCase
{
public:
  OpenfecRsDecodeTestCase ();
  virtual ~OpenfecRsDecodeTestCase ();
  const int symbolSize = 16;
  const double codeRate = 0.5;
  const int packetSize = 1000;

private:
  virtual void DoRun (void);
  ObjectFactory m_codecFactory;
};

#endif /* TEST_AL_FEC_CODEC_OPENFEC_RS_H */
