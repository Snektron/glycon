#include "debugger.h"
#include "command.h"
#include "parser.h"
#include "bdbp_util.h"
#include "glycon.h"
#include "commands/commands.h"
#include "bdbp/binary_debug_protocol.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

void debugger_init(struct debugger* dbg) {
    dbg->quit = false;
    conn_init(&dbg->conn);
    dbg->scratch = malloc(GLYCON_ADDRSPACE_SIZE);
}

void debugger_deinit(struct debugger* dbg) {
    conn_close(&dbg->conn);
    free(dbg->scratch);
}

void debugger_do_line(struct debugger* dbg, size_t len, const char line[len]) {
    struct parser p;
    parser_init(&p, len, line);

    struct cmd_parse_result result;
    if (cmd_parse(&result, &p, commands) && result.matched_command) {
        switch (result.matched_command->type) {
            case CMD_TYPE_LEAF: {
                command_handler_t handler = result.matched_command->leaf.payload;
                handler(dbg, &result);
                break;
            }
            case CMD_TYPE_DIRECTORY:
                // TODO: Print help for the directory itself?
                break;
        }
    }

    cmd_parse_result_deinit(&result);
}

void debugger_repl(struct debugger* dbg) {
    rl_initialize();
    dbg->quit = false;

    while (!dbg->quit) {
        char* line = readline(prompt);
        if (!line) {
            return;
        } else {
            size_t len = strlen(line);
            debugger_do_line(dbg, len, line);
            free(line);
        }
    }
    rl_uninitialize();
}

bool debugger_require_connection(struct debugger* dbg) {
    if (!conn_is_open(&dbg->conn)) {
        debugger_print_error(dbg, "No active connection. Connect to a device using `connection open`.");
        return true;
    }

    return false;
}

bool debugger_exec_cmd(struct debugger* dbg, uint8_t* buf) {
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

    enum bdbp_status status = result;
    if (status != BDBP_STATUS_SUCCESS) {
        debugger_print_error(dbg, "Device returned status %s.", bdbp_status_to_string(status));
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

    return false;
}

void debugger_print_error(struct debugger* dbg, const char* fmt, ...) {
    (void) dbg;
    va_list args;
    va_start(args, fmt);
    printf("error: ");
    vprintf(fmt, args);
    puts("");
    va_end(args);
}

bool debugger_write_memory(struct debugger* dbg, uint16_t address, size_t len, const uint8_t buffer[len]) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    for (size_t i = 0; i < len;) {
        bdbp_pkt_init(pkt, BDBP_CMD_WRITE);
        bdbp_pkt_append_u16(pkt, address + i);
        uint8_t cap = bdbp_pkt_data_free(pkt);
        size_t bytes_left = len - i;
        uint8_t bytes_in_pkt = cap < bytes_left ? cap : bytes_left;
        bdbp_pkt_append_data(pkt, bytes_in_pkt, &buffer[i]);
        i += bytes_in_pkt;
        if (debugger_exec_cmd(dbg, pkt))
            return true;
    }

    return true;
}

bool debugger_read_memory(struct debugger* dbg, uint16_t address, size_t len, uint8_t buffer[len]) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    for (size_t i = 0; i < len; i += BDBP_MAX_DATA_LENGTH) {
        bdbp_pkt_init(pkt, BDBP_CMD_READ);
        bdbp_pkt_append_u16(pkt, address + i);
        size_t bytes_left = len - i;
        uint8_t bytes_in_pkt = BDBP_MAX_DATA_LENGTH < bytes_left ? BDBP_MAX_DATA_LENGTH : bytes_left;
        bdbp_pkt_append_u8(pkt, bytes_in_pkt);
        if (debugger_exec_cmd(dbg, pkt))
            return true;

        memcpy(&buffer[i], &pkt[BDBP_FIELD_DATA], bytes_in_pkt);
    }

    return false;
}
