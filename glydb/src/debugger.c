#include "debugger.h"
#include "command.h"
#include "parser.h"
#include "bdbp_util.h"
#include "commands/commands.h"
#include "bdbp/binary_debug_protocol.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

void debugger_init(struct debugger* dbg) {
    dbg->quit = false;
    conn_init(&dbg->conn);
}

void debugger_deinit(struct debugger* dbg) {
    conn_close(&dbg->conn);
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
}

bool debugger_require_connection(struct debugger* dbg) {
    if (!conn_is_open(&dbg->conn)) {
        printf("error: No active connection. Connect to a device using `connection open`.\n");
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
        printf("write error: %s.\n", strerror(errno));
        return true;
    }

    // TODO: Improve this to ideally a single read call
    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        printf("read error A: %s.\n", strerror(errno));
        return true;
    }

    enum bdbp_status status = result;
    if (status != BDBP_STATUS_SUCCESS) {
        printf("device error: %s.\n", bdbp_status_to_string(status));
        return true;
    }
    buf[0] = result;

    result = conn_read_byte(&dbg->conn);
    // TODO: Remove this code duplication also
    if (result < 0) {
        printf("read error B: %s.\n", strerror(errno));
        return true;
    }
    buf[1] = result;

    len = result;
    for (size_t i = 0; i < len; ++i) {
        result = conn_read_byte(&dbg->conn);
        // TODO: Remove this code duplication also
        if (result < 0) {
            printf("read error C: %s.\n", strerror(errno));
            return true;
        }
        buf[i + 2] = result;
    }

    return false;
}
