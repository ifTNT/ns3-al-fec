#ifndef AL_FEC_CODEC_H
#define AL_FEC_CODEC_H

#include "ns3/packet.h"
#include <optional>

namespace ns3 {

class AlFecCodec
{
public:
  /**
   * \brief Specify the source block
  */
  virtual void SetSourceBlock (Packet p) = 0;

  /**
   * \brief Get the next encoded symbol
   * 
   * \return If there's unsent encoded symbol, return the packet with encode header.
   * Other, return std::nullopt
  */
  virtual std::optional<Packet> NextEncodedSymbol () = 0;

  /**
   * \brief Decode source block with received symbol
   * 
   * \return If the source block successfully decoded, return the decoded block.
   * Other, return std::nullopt
  */
  virtual std::optional<Packet> Decode (Packet p) = 0;
};

} // namespace ns3

#endif // AL_FEC_CODEC_H