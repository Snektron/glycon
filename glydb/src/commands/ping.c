#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void ping(struct debugger* dbg, const struct cmd_parse_result* args) {
    if (debugger_require_connection(dbg))
        return;

    uint8_t buf[2];
    bdbp_pkt_init(buf, BDBP_CMD_PING);
    if (debugger_exec_cmd(dbg, buf))
        return;
    puts("pong!");
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
