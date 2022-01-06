#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_WRITE);
    bdbp_pkt_append_u16(buf, address);
    bdbp_pkt_append_u8(buf, data);
    debugger_exec_cmd(dbg, buf);
}

static void memory_read(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_READ);
    bdbp_pkt_append_u16(buf, address);
    bdbp_pkt_append_u8(buf, 1);
    if (debugger_exec_cmd(dbg, buf))
       return;

    printf("%d 0x%02X\n", buf[0], buf[0]);
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Help for 'memory write'.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "values", "Value to write."},
            {}
        },
        .payload = memory_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Help for 'memory read'.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to read from."},
            {}
        },
        .payload = memory_read
    }}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Help for 'memory'.",
    {.directory = {memory_commands}}
};
