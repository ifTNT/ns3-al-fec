/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <stdint.h>
#include "al-fec-info-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlFecInfoTag");

AlFecInfoTag::AlFecInfoTag () : m_k (0), m_packetContext (Buffer ())
{
  NS_LOG_FUNCTION (this);
}

void
AlFecInfoTag::SetPacketContext (Buffer context)
{

  m_packetContext = context;
}

Buffer
AlFecInfoTag::GetPacketContext ()
{
  return m_packetContext;
}

void
AlFecInfoTag::SetK (uint16_t k)
{
  NS_LOG_FUNCTION (this << k);
  m_k = k;
}

uint16_t
AlFecInfoTag::GetK () const
{
  return m_k;
}

void
AlFecInfoTag::SetSymbolSize (uint8_t symbolSize)
{
  NS_LOG_FUNCTION (this << symbolSize);
  m_symbolSize = symbolSize;
}

uint8_t
AlFecInfoTag::GetSymbolSize () const
{
  return m_symbolSize;
}

TypeId
AlFecInfoTag::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::AlFecInfoTag").SetParent<Tag> ().AddConstructor<AlFecInfoTag> ();
  return tid;
}
TypeId
AlFecInfoTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

uint32_t
AlFecInfoTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return sizeof (m_k) + sizeof (m_symbolSize) + sizeof (uint32_t) + m_packetContext.GetSize ();
}
void
AlFecInfoTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);

  uint32_t packetContextSize = m_packetContext.GetSize ();

  i.WriteU16 (m_k);
  i.WriteU8 (m_symbolSize);
  i.WriteU32 (packetContextSize);
  i.Write (m_packetContext.PeekData (), packetContextSize);
}
void
AlFecInfoTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);

  uint32_t packetContextSize;
  uint8_t *buf;
  m_k = i.ReadU16 ();
  m_symbolSize = i.ReadU8 ();
  packetContextSize = i.ReadU32 ();
  m_packetContext = Buffer ();
  m_packetContext.AddAtStart (packetContextSize);

  buf = new uint8_t[packetContextSize];
  i.Read (buf, packetContextSize);
  m_packetContext.Begin ().Write (buf, packetContextSize);

  delete[] buf;
}
void
AlFecInfoTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "AlFecInfoTag [K=" << (int) m_k;
  os << ", Symbol size:" << (int) m_symbolSize;
  os << ", Size of packet context:" << (int) m_packetContext.GetSize ();
  os << "] ";
}
} // namespace ns3
