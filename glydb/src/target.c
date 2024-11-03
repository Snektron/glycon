#include "target.h"
#include "bdbp_util.h"

#include "common/binary_debug_protocol.h"
#include "common/glycon.h"

#include <string.h>
#include <errno.h>

bool target_exec_cmd(struct debugger* dbg, uint8_t* buf) {
    if (debugger_require_connection(dbg))
        return true;

    size_t len = BDBP_MIN_MSG_LENGTH + buf[BDBP_FIELD_DATA_LEN];

    int result = conn_write_all(&dbg->conn, len, buf);
    if (result < 0) {
        debugger_print_error(dbg, "Failed to write: %s.", strerror(errno));
        return true;
    }

    // TODO: Improve this to ideally a single read call
    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        debugger_print_error(dbg, "Failed to read: %s.", strerror(errno));
        return true;
    }
    buf[0] = result;

    result = conn_read_byte(&dbg->conn);
    // TODO: Remove this code duplication also
    if (result < 0) {
        debugger_print_error(dbg, "Failed to read: %s.", strerror(errno));
        return true;
    }
    buf[1] = result;

    len = result;
    for (size_t i = 0; i < len; ++i) {
        result = conn_read_byte(&dbg->conn);
        // TODO: Remove this code duplication also
        if (result < 0) {
            debugger_print_error(dbg, "Failed to read: %s.", strerror(errno));
            return true;
        }
        buf[i + 2] = result;
    }

    enum bdbp_status status = buf[0];
    if (status != BDBP_STATUS_SUCCESS) {
        debugger_print_error(dbg, "Device returned status %s.", bdbp_status_to_string(status));
        return true;
    }

    return false;
}

static bool target_write(struct debugger* dbg, enum bdbp_cmd cmd, gly_addr_t address, size_t len, const uint8_t buffer[]) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    for (size_t i = 0; i < len;) {
        bdbp_pkt_init(pkt, cmd);
        bdbp_pkt_append_addr(pkt, address + i);
        uint8_t cap = bdbp_pkt_data_free(pkt);
        size_t bytes_left = len - i;
        uint8_t bytes_in_pkt = cap < bytes_left ? cap : bytes_left;
        bdbp_pkt_append_data(pkt, bytes_in_pkt, &buffer[i]);
        i += bytes_in_pkt;
        if (target_exec_cmd(dbg, pkt))
            return true;
    }

    return true;
}

bool target_write_memory(struct debugger* dbg, gly_addr_t address, size_t len, const uint8_t buffer[]) {
    return target_write(dbg, BDBP_CMD_WRITE, address, len, buffer);
}

bool target_write_flash(struct debugger* dbg, gly_addr_t address, size_t len, const uint8_t buffer[]) {
    return target_write(dbg, BDBP_CMD_WRITE_FLASH, address, len, buffer);
}

bool target_read_memory(struct debugger* dbg, gly_addr_t address, size_t len, uint8_t buffer[]) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    for (size_t i = 0; i < len; i += BDBP_MAX_DATA_LENGTH) {
        bdbp_pkt_init(pkt, BDBP_CMD_READ);
        bdbp_pkt_append_addr(pkt, address + i);
        size_t bytes_left = len - i;
        uint8_t bytes_in_pkt = BDBP_MAX_DATA_LENGTH < bytes_left ? BDBP_MAX_DATA_LENGTH : bytes_left;
        bdbp_pkt_append_u8(pkt, bytes_in_pkt);
        if (target_exec_cmd(dbg, pkt))
            return true;

        memcpy(&buffer[i], &pkt[BDBP_FIELD_DATA], bytes_in_pkt);
    }

    return false;
}
