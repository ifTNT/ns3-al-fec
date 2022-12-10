#include "util.h"

#include <iomanip>
#include <unistd.h>
#include <fcntl.h>

void
fillRandomBytes (uint8_t *buf, unsigned int size)
{
  int fd = open ("/dev/random", O_RDONLY);
  read (fd, buf, size);
}

std::string
printBuffer (uint8_t *buf, unsigned int size)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < size; i++)
    {
      ss << std::setfill ('0') << std::setw (2) << std::right << std::hex
         << static_cast<unsigned int> (*(buf + i)) << " ";
    }
  ss << std::endl;
  return ss.str ();
}