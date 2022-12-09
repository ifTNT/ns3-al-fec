#ifndef AL_FEC_INFO_TAG_H
#define AL_FEC_INFO_TAG_H

#include "ns3/tag.h"
#include "ns3/tag-buffer.h"
#include "ns3/packet.h"

namespace ns3 {
/**
 * \brief Stores all the context of the original packet except the content buffer.
 * 
 * \warning The implementation follows the ns3::Packet.
 * If the serialization order is changed, this module won't work correctly.
*/
class AlFecInfoTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer buf) const;
  virtual void Deserialize (TagBuffer buf);
  virtual void Print (std::ostream &os) const;
  AlFecInfoTag ();

  void SetPacketContext (Buffer context);

  Buffer GetPacketContext ();

  /**
   * \brief Set the number of source symbol
   *
   * \param k The number of source symbol
   */
  void SetK (uint16_t k);

  /**
   * \brief Get the number of source symbol
   *
   * \returns The number of source symbol
   */
  uint16_t GetK () const;

  void SetSymbolSize(uint8_t symbolSize);
  uint8_t GetSymbolSize () const;


private:
  uint16_t m_k; // Number of the source symbol
  uint8_t m_symbolSize; // Symbol size
  Buffer m_packetContext;
};

} // namespace ns3

#endif // AL_FEC_INFO_TAG_H