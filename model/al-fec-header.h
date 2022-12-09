#ifndef AL_FEC_HEADER_H
#define AL_FEC_HEADER_H

#include "ns3/header.h"

#include <list>

namespace ns3::AlFecHeader {

class EncodeHeader : public Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  EncodeHeader ();
  ~EncodeHeader ();

  /**
   * \brief Set Encoded Symbol ID (ESI)
   *
   * \param esi ESI to set
   */
  void SetEncodedSymbolId (uint8_t esi);

  /**
   * \brief Set Sequence Number
   *
   * \param sn Sequence Number to set
   */
  // void SetSequenceNumber (uint16_t sn);

  /**
   * \brief Get Encoded Symbol ID (ESI)
   *
   * \returns ESI
   */
  uint8_t GetEncodedSymbolId () const;

  /**
   * \brief Get the Sequence Number of source block
   *
   * \returns The Sequence Number of source block
   */
  // uint16_t GetSequenceNumber () const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint16_t m_esi; // Encoded symbol id
  // uint16_t m_k; // Number of the source symbol
  // uint16_t m_sn; // Sequence number of source packet
};

class PayloadHeader : public Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  PayloadHeader ();
  ~PayloadHeader ();

  /**
   * \brief Set the size of source packet padding
   *
   * \param size the size of source packet padding to set
   */
  void SetPaddingSize (uint8_t paddingSize);

  /**
   * \brief Get the size of source packet padding
   *
   * \returns The the size of source packet padding
   */
  uint8_t GetPaddingSize () const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint8_t m_paddingSize; // The size of source packet padding
};

} // namespace ns3::AlFecHeader

#endif // AL_FEC_HEADER_H
