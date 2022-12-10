/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/log.h"

#include "ns3/al-fec-header.h"

namespace ns3::AlFecHeader {

NS_LOG_COMPONENT_DEFINE ("AlFecHeader");

NS_OBJECT_ENSURE_REGISTERED (EncodeHeader);

EncodeHeader::EncodeHeader () : m_esi (0) //, m_sn (0)
{
}

EncodeHeader::~EncodeHeader ()
{
  m_esi = 0;
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

  os << "ESI=" << (int) m_esi;
}

uint32_t
EncodeHeader::GetSerializedSize (void) const
{
  return sizeof(m_esi); //+ sizeof(m_sn)
}

void
EncodeHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // i.WriteHtonU16 (m_sn);
  i.WriteHtonU16 (m_esi);
}

uint32_t
EncodeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  // m_sn = i.ReadNtohU16 ();
  m_esi = i.ReadNtohU16 ();

  return GetSerializedSize ();
}

/*=======================*
 *     PayloadHeader     *
 *=======================*/

NS_OBJECT_ENSURE_REGISTERED (PayloadHeader);

PayloadHeader::PayloadHeader () : m_paddingSize (0)
{
}

PayloadHeader::~PayloadHeader ()
{
  m_paddingSize = 0;
}

void
PayloadHeader::SetPaddingSize (uint8_t size)
{
  m_paddingSize = size;
}

uint8_t
PayloadHeader::GetPaddingSize () const
{
  return m_paddingSize;
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
  os << "PaddingSize=" << (int) m_paddingSize;
}

uint32_t
PayloadHeader::GetSerializedSize (void) const
{
  return sizeof(m_paddingSize);
}

void
PayloadHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_paddingSize);
}

uint32_t
PayloadHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_paddingSize = i.ReadU8 ();

  return GetSerializedSize ();
}

}; // namespace ns3::AlFecHeader
