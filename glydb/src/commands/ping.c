#include "commands/commands.h"
#include "debugger.h"
#include "protocol.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void ping(struct debugger* dbg, size_t len, const char* const args[len]) {
    (void) len;
    (void) args;
    if (debugger_require_connection(dbg))
        return;

    int result = conn_write_byte(&dbg->conn, GLYCO_CMD_PING);
    if (result < 0) {
        printf("write error: %s.\n", strerror(errno));
        return;
    }

    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        printf("read error: %s.\n", strerror(errno));
    } else {
        printf("device returned: %s.\n", glyco_status_to_string((enum glyco_status) result));
    }
}

const struct cmd command_ping = {
    .type = CMD_TYPE_LEAF,
    .name = "ping",
    .help = "Ping the currently connected device.",
    {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = ping
    }
}};
