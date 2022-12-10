#ifndef AL_FEC_UTIL_H
#define AL_FEC_UTIL_H

#include <string>
#include <stdint.h>

void fillRandomBytes (uint8_t *buf, unsigned int size);
std::string printBuffer (uint8_t *buf, unsigned int size);

#endif // AL_FEC_UTIL_H