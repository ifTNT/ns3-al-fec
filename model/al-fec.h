#ifndef AL_FEC_H
#define AL_FEC_H

#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/al-fec-codec.h"
#include <optional>

namespace ns3 {

/**
 * \brief The main API of AL-FEC.
 * Internally, it deal with packet encapsulation and interpretation.
 * It does not response to encode and decode.
*/
class AlFec : public Object
{
public:
  AlFec ();
  AlFec (AlFecCodec* codec);
  ~AlFec ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * \brief Assign a new codec
  */
  void SetCodec(AlFecCodec* codec);

  /**
   * \brief Specify the original packet to be encoded
   * 
   * \return The number of resulting packet
  */
  size_t EncodePacket (Ptr<Packet> p);

  /**
   * \brief Get the next packet of encoded block
   * 
   * \return If there's unsent encoded block, return the packet with encode header.
   * Other, return std::nullopt
  */
  std::optional<Ptr<Packet>> NextEncodedPacket ();

  /**
   * \brief Try to decode original packet with received packet
   * 
   * \return If the source packet successfully decoded, return the decoded packet.
   * Other, return std::nullopt
  */
  std::optional<Ptr<Packet>> DecodePacket (Ptr<Packet> p);

private:
  Ptr<Packet> m_originalPacket;
  Buffer m_sourceContext;
  Buffer m_sourceBlock;
  AlFecCodec* m_codec;
};

} // namespace ns3

#endif // AL_FEC_CODEC_H