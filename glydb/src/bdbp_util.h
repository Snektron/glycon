#ifndef _GLYDB_BDBP_UTIL_H
#define _GLYDB_BDBP_UTIL_H

#include "common/glycon.h"
#include "common/binary_debug_protocol.h"

#include <stdint.h>
#include <stddef.h>

// This file implements some utility functions for handling BDBP-packets.

// Convert a BDBP-status to a human-readable string.
const char* bdbp_status_to_string(enum bdbp_status status);

// Initialize an empty packet with a particular command type.
void bdbp_pkt_init(uint8_t* pkt, enum bdbp_cmd cmd);

// Return the number of bytes currently in the data part of this packet.
uint8_t bdbp_pkt_data_size(const uint8_t* pkt);
// Return the number of bytes that can still be appended to the data part of this packet.
uint8_t bdbp_pkt_data_free(const uint8_t* pkt);

// Write some data into the data part of this packet.
void bdbp_pkt_append_data(uint8_t* pkt, size_t len, const void* data);

// Write a single 8-bit integer into the data part of a packet.
void bdbp_pkt_append_u8(uint8_t* pkt, uint8_t data);
// Write a single address into the data part of a packet.
void bdbp_pkt_append_addr(uint8_t* pkt, gly_addr_t data);

#endif
