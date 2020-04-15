#ifndef __SECURE_H_
#define __SECURE_H_

#include <stdint.h>

uint8_t crc8(uint8_t *data, int size);

uint8_t Crc8_cal(const void* vptr, int len);

#endif
