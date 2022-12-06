#ifndef AL_FEC_CODEC_OPENFEC_H
#define AL_FEC_CODEC_OPENFEC_H

#include "ns3/al-fec-codec.h"
#include "ns3/object.h"

extern "C" {
#include "openfec/lib_common/of_openfec_api.h"
}

namespace ns3 {

class AlFecCodecOpenfecRs : public Object, public AlFecCodec
{
public:
  AlFecCodecOpenfecRs ();
  ~AlFecCodecOpenfecRs ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * \brief Specify the source block
   * 
   * \return {The number of encoded block (n), the number of source block (k)}
  */
  std::pair<size_t, size_t> SetSourceBlock (Buffer p);


  /**
   * \brief Get the next encoded symbol
   * 
   * \return Return the next unsent encoded block.
   * If there's no unsent encoded block, return std::nullopt
  */
  std::optional<std::pair<unsigned int, Buffer>> NextEncodedBlock ();

  /**
   * \brief Decode source block with received block
   * 
   * \param p The content of received block
   * \param esi The received Encoded Symbol ID
   * 
   * \return If the source block successfully decoded, return the decoded block.
   * Other, return std::nullopt
  */
  std::optional<Buffer> Decode (Buffer p, unsigned int esi);

private:
  // Common
  of_session_t *m_session;
  of_rs_2_m_parameters_t m_param;
  uint16_t m_rsM = 8; // RS over GF(2^m). For configuration.
  uint32_t m_symbolSize = 8; // The symbol size. For configuration.
  double m_codeRate = 0.5; // Code rate. For configuration.
  const of_codec_id_t m_codecId = OF_CODEC_REED_SOLOMON_GF_2_M_STABLE;
  std::optional<Buffer> m_sourceBlock;
  const int m_sizeOfLenField = sizeof (unsigned int);

  // Encode
  unsigned int m_esi; // Current ESI
  uint8_t **m_encodedSymbol; // Table of encoded symbol

  // Decode
  uint8_t **m_sourceSymbol; // Table of decoded source symbol
};

} // namespace ns3

#endif // AL_FEC_CODEC_OPENFEC_H
