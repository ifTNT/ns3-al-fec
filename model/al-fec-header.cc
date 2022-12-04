/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "ns3/al-fec-header.h"

namespace ns3::AlFecHeader {

NS_LOG_COMPONENT_DEFINE ("AlFecHeader");

NS_OBJECT_ENSURE_REGISTERED (EncodeHeader);

EncodeHeader::EncodeHeader () : m_esi (0), m_k (0) //, m_sn (0)
{
}

EncodeHeader::~EncodeHeader ()
{
  m_esi = 0;
  m_k = 0;
  // m_sn = 0;
}

void
EncodeHeader::SetEncodedSymbolId (uint8_t esi)
{
  m_esi = esi;
}

uint8_t
EncodeHeader::GetEncodedSymbolId () const
{
  return m_esi;
}

void
EncodeHeader::SetK (uint8_t k)
{
  m_k = k;
}

uint8_t
EncodeHeader::GetK () const
{
  return m_k;
}

// void
// PayloadHeader::SetSequenceNumber (uint16_t sn)
// {
//   m_sn = sn;
// }

// uint16_t
// PayloadHeader::GetSequenceNumber () const
// {
//   return m_sn;
// }

TypeId
EncodeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AlFecHeader::EncodeHeader")
                          .SetParent<Header> ()
                          .AddConstructor<EncodeHeader> ();
  return tid;
}

TypeId
EncodeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
EncodeHeader::Print (std::ostream &os) const
{
  // os << "SN=" << (int) m_sn;
  os << "K=" << (int) m_k;
  os << " ESI=" << (int) m_esi;
}

uint32_t
EncodeHeader::GetSerializedSize (void) const
{
  return sizeof(m_k)+sizeof(m_esi);
}

void
EncodeHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // i.WriteHtonU16 (m_sn);
  i.WriteU8 (m_k);
  i.WriteU8 (m_esi);
}

uint32_t
EncodeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  // m_sn = i.ReadNtohU16 ();
  m_k = i.ReadU8 ();
  m_esi = i.ReadU8 ();

  return GetSerializedSize ();
}

/*=======================*
 *     PayloadHeader     *
 *=======================*/

NS_OBJECT_ENSURE_REGISTERED (PayloadHeader);

PayloadHeader::PayloadHeader () : m_size (0)
{
}

PayloadHeader::~PayloadHeader ()
{
  m_size = 0;
}

void
PayloadHeader::SetSize (uint16_t size)
{
  m_size = size;
}

uint16_t
PayloadHeader::GetSize () const
{
  return m_size;
}

TypeId
PayloadHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AlFecHeader::PayloadHeader")
                          .SetParent<Header> ()
                          .AddConstructor<PayloadHeader> ();
  return tid;
}

TypeId
PayloadHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
PayloadHeader::Print (std::ostream &os) const
{
  os << "Size=" << (int) m_size;
}

uint32_t
PayloadHeader::GetSerializedSize (void) const
{
  return sizeof(m_size);
}

void
PayloadHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_size);
}

uint32_t
PayloadHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_size = i.ReadNtohU16 ();

  return GetSerializedSize ();
}

}; // namespace ns3::AlFecHeader
