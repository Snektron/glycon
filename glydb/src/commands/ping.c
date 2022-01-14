#include "commands/commands.h"
#include "debugger.h"
#include "bdbp_util.h"
#include "target.h"

#include "common/binary_debug_protocol.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static void ping(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(pkt, BDBP_CMD_PING);
    if (target_exec_cmd(dbg, pkt))
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
