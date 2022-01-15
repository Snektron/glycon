#include "commands/commands.h"
#include "debugger.h"
#include "bdbp_util.h"
#include "target.h"

#include "common/glycon.h"
#include "common/binary_debug_protocol.h"

#include <stdlib.h>
#include <stdio.h>

// TODO: handle multiple
static void flash_write_op(struct debugger* dbg, const struct debugger_write_op* op) {
    if (!glycon_is_flash_addr(op->address)) {
        debugger_print_error(dbg, "Base address does not lie within flash address space. Use `memory write` to write to ram.");
        return;
    } else if (op->address + op->len > GLYCON_FLASH_END) {
        debugger_print_error(dbg, "Write overflows flash address space.");
    }

    target_write_flash(dbg, op->address, op->len, dbg->scratch);
}

static void flash_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    struct debugger_write_op op;
    if (subcommand_write(dbg, args, &op, dbg->scratch))
        return;

    flash_write_op(dbg, &op);
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

static void flash_load(struct debugger* dbg, const struct cmd_parse_result* args) {
    struct debugger_write_op* ops;
    if (subcommand_load(dbg, args, &ops, dbg->scratch))
        return;

    flash_write_op(dbg, &ops[0]);
    free(ops);
}

static void flash_program(struct debugger* dbg, struct cmd_parse_result* args) {
    struct debugger_write_op* ops;
    if (subcommand_load(dbg, args, &ops, dbg->scratch))
        return;

    // TODO: Verify file before erasing?
    // TODO: Move this to target.c
    for (size_t address = GLYCON_FLASH_START; address < GLYCON_FLASH_END; address += GLYCON_FLASH_SECTOR_SIZE) {
        uint8_t pkt[BDBP_MAX_MSG_LENGTH];
        bdbp_pkt_init(pkt, BDBP_CMD_ERASE_SECTOR);
        bdbp_pkt_append_u16(pkt, address);
        if (target_exec_cmd(dbg, pkt)) {
            goto free_ops;
        }
    }

    flash_write_op(dbg, &ops[0]);
free_ops:
    free(ops);
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
    &(struct cmd){CMD_TYPE_LEAF, "load", "Load a file and write it to target flash. Does not erase sectors.", {.leaf = {
        .options = subcommand_load_opts,
        .positionals = subcommand_load_pos,
        .payload = flash_load
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "program", "Erase flash address space and load file.", {.leaf = {
        .options = subcommand_load_opts,
        .positionals = subcommand_load_pos,
        .payload = flash_program
    }}},
    NULL
};

const struct cmd command_flash = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "flash",
    .help = "Functionality related to the target flash chip.",
    {.directory = {flash_commands}}
};
