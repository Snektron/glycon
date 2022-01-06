#include "bdbp_util.h"

#include <string.h>
#include <assert.h>

const char* bdbp_status_to_string(enum bdbp_status status) {
    switch (status) {
        case BDBP_STATUS_SUCCESS:
            return "Success";
        case BDBP_STATUS_UNKNOWN_CMD:
            return "Unknown command";
        default:
            return "(Invalid status)";
    }
}

void bdbp_pkt_init(uint8_t* pkt, enum bdbp_cmd cmd) {
    pkt[BDBP_FIELD_HDR] = cmd;
    pkt[BDBP_FIELD_DATA_LEN] = 0;
}

void bdbp_pkt_append_data(uint8_t* pkt, size_t len, const void* data) {
    size_t current_len = pkt[BDBP_FIELD_DATA_LEN];
    assert(current_len + len <= BDBP_MAX_DATA_LENGTH);
    pkt[BDBP_FIELD_DATA_LEN] = current_len + len;
    memcpy(&pkt[current_len], data, len);
}

