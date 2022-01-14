#include "commands/commands.h"
#include "debugger.h"
#include "bdbp_util.h"

#include "common/binary_debug_protocol.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void ping(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t buf[BDBP_MAX_MSG_LENGTH];
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
