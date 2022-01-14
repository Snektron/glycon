#include "commands/commands.h"
#include "debugger.h"
#include "bdbp_util.h"
#include "target.h"

#include "common/glycon.h"
#include "common/binary_debug_protocol.h"

#include <stdio.h>

static void flash_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;

    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(pkt, BDBP_CMD_WRITE_FLASH);
    bdbp_pkt_append_u16(pkt, address);
    bdbp_pkt_append_u8(pkt, data);
    target_exec_cmd(dbg, pkt);
}

static void flash_info(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(pkt, BDBP_CMD_FLASH_ID);
    if (target_exec_cmd(dbg, pkt))
       return;

    uint8_t mfg = pkt[BDBP_FIELD_DATA + 0];
    uint8_t dev = pkt[BDBP_FIELD_DATA + 1];

    printf("Manufacterer ID: %02X (%s)\n", mfg, mfg == 0xBF ? "ok" : "incorrect");
    printf("Device ID: %02X (%s)\n", dev, dev == 0xB5 ? "ok" : "incorrect");
}

static void flash_erase_sector(struct debugger* dbg, const struct cmd_parse_result* args) {
    int64_t address = args->positionals[0].as_int;
    if (address < 0 || address > GLYCON_ADDRSPACE_SIZE || !glycon_is_flash_addr(address)) {
        debugger_print_error(dbg, "Address %ld does not lie in flash storage.", address);
        return;
    }

    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(pkt, BDBP_CMD_ERASE_SECTOR);
    bdbp_pkt_append_u16(pkt, address);
    target_exec_cmd(dbg, pkt);
}

static void flash_erase_chip(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint8_t pkt[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(pkt, BDBP_CMD_ERASE_CHIP);
    target_exec_cmd(dbg, pkt);
}

static const struct cmd* erase_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "sector", "Erase a sector of the target flash chip.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "Erase the sector containing this address. Sectors are 4KiB, the address will be rounded"},
            {}
        },
        .payload = flash_erase_sector
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "chip", "Erase the entire target flash chip.", {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = flash_erase_chip
    }}},
    NULL
};

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
    &(struct cmd){CMD_TYPE_LEAF, "info", "Print information about the flash chip, such as manufacterer and device ID.", {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = flash_info
    }}},
    &(struct cmd){CMD_TYPE_DIRECTORY, "erase", "Erase (parts of) the target flash chip.", {.directory = {erase_commands}}},
    NULL
};

const struct cmd command_flash = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "flash",
    .help = "Functionality related to the target flash chip.",
    {.directory = {flash_commands}}
};
