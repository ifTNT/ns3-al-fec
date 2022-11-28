#ifndef AL_FEC_HEADER_H
#define AL_FEC_HEADER_H

#include "ns3/header.h"

#include <list>

namespace ns3 {

class AlFecHeader : public Header
{
public:

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  AlFecHeader ();
  ~AlFecHeader ();

  /**
   * \brief Set Encoded Symbol ID (ESI)
   *
   * \param esi ESI to set
   */
  void SetEncodedSymbolId (uint8_t esi);
  /**
   * \brief Set the number of source symbol
   *
   * \param k The number of source symbol
   */
  void SetK (uint8_t k);

  /**
   * \brief Set the sequnce number of source block
   *
   * \param sn The sequnce number of source block
   */
  // void SetSequenceNumber (uint16_t sn);

  /**
   * \brief Get Encoded Symbol ID (ESI)
   *
   * \returns ESI
   */
  uint8_t GetEncodedSymbolId () const;
  /**
   * \brief Get the number of source symbol
   *
   * \returns The number of source symbol
   */
  uint8_t GetK () const;

  /**
   * \brief Get the sequnce number of source block
   *
   * \returns The sequnce number of source block
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
  uint8_t m_esi; // Encoded symbol id
  uint8_t m_k;   // Number of the source symbol
  // uint16_t m_sn;  // Sequence number of source block

};

} // namespace ns3

#endif // AL_FEC_HEADER_H
