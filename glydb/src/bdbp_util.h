#ifndef _GLYDB_BDBP_UTIL_H
#define _GLYDB_BDBP_UTIL_H

#include <stdint.h>
#include <stddef.h>

#include "bdbp/binary_debug_protocol.h"

const char* bdbp_status_to_string(enum bdbp_status status);

void bdbp_pkt_init(uint8_t* pkt, enum bdbp_cmd cmd);

uint8_t bdbp_pkt_data_size(const uint8_t* pkt);
uint8_t bdbp_pkt_data_free(const uint8_t* pkt);

void bdbp_pkt_append_data(uint8_t* pkt, size_t len, const void* data);

void bdbp_pkt_append_u8(uint8_t* pkt, uint8_t data);
void bdbp_pkt_append_u16(uint8_t* pkt, uint16_t data);

#endif
