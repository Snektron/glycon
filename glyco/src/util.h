#ifndef GLYCO_SRC_UTIL_H
#define GLYCO_SRC_UTIL_H

#include <stdint.h>

static inline uint8_t util_bit_reverse8(uint8_t x) {
    x = (x & 0xF0) >> 4 | (x & 0x0F) << 4;
    x = (x & 0xCC) >> 2 | (x & 0x33) << 2;
    x = (x & 0xAA) >> 1 | (x & 0x55) << 1;
    return x;
}

#endif
