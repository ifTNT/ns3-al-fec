/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "ns3/al-fec-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AlFecHeader");

NS_OBJECT_ENSURE_REGISTERED (AlFecHeader);

AlFecHeader::AlFecHeader () : m_esi (0), m_k (0) //, m_sn (0)
{
}

AlFecHeader::~AlFecHeader ()
{
  m_esi = 0;
  m_k = 0;
  // m_sn = 0;
}

void
AlFecHeader::SetEncodedSymbolId (uint8_t esi)
{
  m_esi = esi;
}

void
AlFecHeader::SetK (uint8_t k)
{
  m_k = k;
}

// void
// AlFecHeader::SetSequenceNumber (uint16_t sn)
// {
//   m_sn = sn;
// }

uint8_t
AlFecHeader::GetEncodedSymbolId () const
{
  return m_esi;
}

uint8_t
AlFecHeader::GetK () const
{
  return m_k;
}

// uint16_t
// AlFecHeader::GetSequenceNumber () const
// {
//   return m_sn;
// }

TypeId
AlFecHeader::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::AlFecHeader").SetParent<Header> ().AddConstructor<AlFecHeader> ();
  return tid;
}

TypeId
AlFecHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
AlFecHeader::Print (std::ostream &os) const
{
  // os << "SN=" << (int) m_sn;
  os << "K=" << (int) m_k;
  os << " ESI=" << (int) m_esi;
}

uint32_t
AlFecHeader::GetSerializedSize (void) const
{
  return 4;
}

void
AlFecHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // i.WriteHtonU16 (m_sn);
  i.WriteU8 (m_k);
  i.WriteU8 (m_esi);
}

uint32_t
AlFecHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  // m_sn = i.ReadNtohU16 ();
  m_k = i.ReadU8 ();
  m_esi = i.ReadU8 ();

  return GetSerializedSize ();
}

}; // namespace ns3
