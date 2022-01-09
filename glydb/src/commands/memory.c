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

    printf("0x%04X: %d 0x%02X\n", address, buf[0], buf[0]);
}

static void memory_flash(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_WRITE_FLASH);
    bdbp_pkt_append_u16(buf, address);
    bdbp_pkt_append_u8(buf, data);
    debugger_exec_cmd(dbg, buf);
}

static void memory_flash_id(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_FLASH_ID);
    if (debugger_exec_cmd(dbg, buf))
       return;

    printf("Manufacterer ID: %02X\n", buf[0]);
    printf("Device ID: %02X\n", buf[1]);
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
    &(struct cmd){CMD_TYPE_LEAF, "flash", "Write to target flash.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "value", "The value to write."},
            {}
        },
        .payload = memory_flash
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "flash-id", "Retrieve flash chip software ID", {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = memory_flash_id
    }}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Help for 'memory'.",
    {.directory = {memory_commands}}
};
