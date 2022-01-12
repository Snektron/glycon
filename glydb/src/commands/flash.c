#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>

static void flash_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_WRITE_FLASH);
    bdbp_pkt_append_u16(buf, address);
    bdbp_pkt_append_u8(buf, data);
    debugger_exec_cmd(dbg, buf);
}

static void flash_id(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_FLASH_ID);
    if (debugger_exec_cmd(dbg, buf))
       return;

    printf("Manufacterer ID: %02X (%s)\n", buf[0], buf[0] == 0xBF ? "ok" : "incorrect");
    printf("Device ID: %02X (%s)\n", buf[1], buf[1] == 0xB5 ? "ok" : "incorrect");
}

static const struct cmd* flash_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Write to target flash.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "value", "The value to write."},
            {}
        },
        .payload = flash_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "id", "Retrieve flash chip software ID", {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = flash_id
    }}},
    NULL
};

const struct cmd command_flash = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "flash",
    .help = "Functionality related to the target flash chip",
    {.directory = {flash_commands}}
};
