#include "ns3/al-fec-codec.h"
#include "ns3/log.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("AlFecCodec");
void
AlFecCodec::SetN (size_t n)
{
  NS_ASSERT_MSG (n >= m_k, "N must greater or equal to K");
  m_n = n;
}
void
AlFecCodec::SetK (size_t k)
{
  NS_ASSERT_MSG (k > 0, "K must greater than 0");
  m_k = k;
}
size_t
AlFecCodec::GetN ()
{
  return m_n;
}
size_t
AlFecCodec::GetK ()
{
  return m_k;
}
} // namespace ns3