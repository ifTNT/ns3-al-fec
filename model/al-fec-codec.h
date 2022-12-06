#ifndef AL_FEC_CODEC_H
#define AL_FEC_CODEC_H

#include "ns3/buffer.h"
#include "ns3/ptr.h"
#include <optional>
#include <utility>

namespace ns3 {

/**
 * \brief The generic encoding and decoding class and interface of AL-FEC.
 * This class does not response to packet encapsulation and interpretation.
*/
class AlFecCodec
{
public:
  /**
   * \brief Specify the source block.
   * The encoder implementation must override this.
   * 
   * \return {The number of encoded block (n), the number of source block (k)}
  */
  virtual std::pair<size_t, size_t> SetSourceBlock (Buffer p) = 0;

  /**
   * \brief Get the next encoded symbol.
   * The encoder implementation must override this.
   * 
   * \return Return the next unsent encoded block.
   * If there's no unsent encoded block, return std::nullopt
  */
  virtual std::optional<std::pair<unsigned int, Buffer>> NextEncodedBlock () = 0;

  /**
   * \brief Decode source block with received block.
   * The decoder implementation must override this.
   * 
   * \param p The content of received block
   * \param esi The received Encoded Symbol ID
   * 
   * \return If the source block successfully decoded, return the decoded block.
   * Other, return std::nullopt
  */
  virtual std::optional<Buffer> Decode (Buffer p, unsigned int esi) = 0;

  void SetN (size_t n);
  void SetK (size_t k);
  size_t GetN ();
  size_t GetK ();

protected:
  size_t m_n = 0; // Number of encoded blocks
  size_t m_k = 0; // Number of source blocks
};

} // namespace ns3

#endif // AL_FEC_CODEC_H