#include <stdint.h>
#include <string.h>
#include "secure.h"

/*
crc8 poly = 0x107 (x8+x2+x1+1)
*/
uint8_t crc8(uint8_t *data, int size)
{
    uint8_t crc = 0x00;
    uint8_t poly = 0x07;
    int bit;

    while (size--)
    {
        crc ^= *data++;
        for (bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}



/* Return CRC-8 of the data, using x^8 + x^2 + x + 1 polynomial.  A
 * table-based algorithm would be faster, but for only a few bytes it isn't
 * worth the code size. */
uint8_t Crc8_cal(const void* vptr, int len) 
{
  const uint8_t *data = vptr;
  unsigned crc = 0;
  int i, j;
  for (j = len; j; j--, data++) {
    crc ^= (*data << 8);
    for(i = 8; i; i--) {
      if (crc & 0x8000)
        crc ^= (0x1070 << 3);
      crc <<= 1;
    }
  }
  return (uint8_t)(crc >> 8);
}

