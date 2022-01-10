#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>

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

    printf("0x%04X: %d 0x%02X\n", address, buf[0], buf[0]);
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Write to target memory.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "value", "Value to write."},
            {}
        },
        .payload = memory_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Read from target memory.", {.leaf = {
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
    .help = "Manipulate target memory",
    {.directory = {memory_commands}}
};
