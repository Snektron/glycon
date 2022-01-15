#include "commands/commands.h"
#include "debugger.h"
#include "bdbp_util.h"
#include "target.h"

#include "common/glycon.h"
#include "common/binary_debug_protocol.h"

#include <stdio.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address;
    size_t len;
    if (subcommand_write(dbg, args, &address, &len, dbg->scratch))
        return;

    if (!glycon_is_ram_addr(address)) {
        debugger_print_error(dbg, "Base address does not lie within ram address space. Use `flash write` to write to flash storage.");
        return;
    } else if (address + len > GLYCON_RAM_END) {
        debugger_print_error(dbg, "Write overflows ram address space.");
    }

    target_write_memory(dbg, address, len, dbg->scratch);
}

static void memory_read(struct debugger* dbg, const struct cmd_parse_result* args) {
    int64_t address = args->positionals[0].as_int;
    if (address < 0 || address > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "Address %ld outside of valid range [0, %d).", address, GLYCON_ADDRSPACE_SIZE);
        return;
    }

    int64_t amt = args->positionals_len > 1 ? args->positionals[1].as_int : 1;
    if (amt < 1 || amt > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "amount %ld outside valid range [1, %d]", amt, GLYCON_ADDRSPACE_SIZE);
        return;
    }

    if (target_read_memory(dbg, address, amt, dbg->scratch))
        return;

    uint8_t bytes_per_line = 16;
    for (size_t i = 0; i < amt; i += bytes_per_line) {
        printf("%04X:", (uint16_t)(address + i));
        for (uint8_t j = 0; j < bytes_per_line && j + i < amt; ++j) {
            printf(" %02X", dbg->scratch[j + i]);
        }
        puts("");
    }
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Write to target memory.", {.leaf = {
        .options = subcommand_write_opts,
        .positionals = subcommand_write_pos,
        .payload = memory_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Read from target memory.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to read from."},
            {VALUE_TYPE_INT, "amount", "The number of bytes to read (default: 1).", CMD_OPTIONAL},
            {}
        },
        .payload = memory_read
    }}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Manipulate target memory.",
    {.directory = {memory_commands}}
};
