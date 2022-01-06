#ifndef _GLYDB_BDBP_UTIL_H
#define _GLYDB_BDBP_UTIL_H

#include <stdint.h>
#include <stddef.h>

#include "bdbp/binary_debug_protocol.h"

const char* bdbp_status_to_string(enum bdbp_status status);

void bdbp_pkt_init(uint8_t* pkt, enum bdbp_cmd cmd);

void bdbp_pkt_append_data(uint8_t* pkt, size_t len, const void* data);

#endif
