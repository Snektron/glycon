#include "bdbp_util.h"

#include <string.h>
#include <assert.h>

const char* bdbp_status_to_string(enum bdbp_status status) {
    switch (status) {
        case BDBP_STATUS_SUCCESS:
            return "Success";
        case BDBP_STATUS_UNKNOWN_CMD:
            return "Unknown command";
        case BDBP_STATUS_BUS_ACQUIRE_TIMEOUT:
            return "Bus acquisition timed out";
        default:
            return "(Invalid status)";
    }
}

void bdbp_pkt_init(uint8_t* pkt, enum bdbp_cmd cmd) {
    pkt[BDBP_FIELD_HDR] = cmd;
    pkt[BDBP_FIELD_DATA_LEN] = 0;
}

uint8_t bdbp_pkt_data_size(const uint8_t* pkt) {
    return pkt[BDBP_FIELD_DATA_LEN];
}

uint8_t bdbp_pkt_data_free(const uint8_t* pkt) {
    return BDBP_MAX_DATA_LENGTH - bdbp_pkt_data_size(pkt);
}

void bdbp_pkt_append_data(uint8_t* pkt, size_t len, const void* data) {
    size_t current_len = pkt[BDBP_FIELD_DATA_LEN];
    assert(current_len + len <= BDBP_MAX_DATA_LENGTH);
    pkt[BDBP_FIELD_DATA_LEN] = current_len + len;
    memcpy(&pkt[BDBP_FIELD_DATA + current_len], data, len);
}

void bdbp_pkt_append_u8(uint8_t* pkt, uint8_t data) {
    bdbp_pkt_append_data(pkt, sizeof(uint8_t), &data);
}

void bdbp_pkt_append_addr(uint8_t* pkt, gly_addr_t data) {
    bdbp_pkt_append_u8(pkt, data & 0xFF);
    bdbp_pkt_append_u8(pkt, (data >> 8) & 0xFF);
    bdbp_pkt_append_u8(pkt, (data >> 16) & 0xF);
}
