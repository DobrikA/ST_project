#ifndef __CRC_CALC_H
#define __CRC_CALC_H

#include <inttypes.h>

uint8_t crc8_calculation(const uint8_t *data, const uint32_t size, const uint8_t poly);

#endif
