#include "commands/commands.h"
#include "debugger.h"
#include "target.h"
#include "z80/z80.h"
#include "z80/disassemble.h"

#include "common/glycon.h"

#include <stdlib.h>
#include <stdio.h>

static void disassemble(struct debugger* dbg, const struct cmd_parse_result* args) {
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

    size_t offset = 0;
    while (offset < amt) {
        struct z80_inst inst;
        printf("%04X:  ", (uint16_t)(address + offset));
        z80_disassemble(&inst, amt - offset, dbg->scratch + offset);

        for (size_t i = 0; i < inst.size; ++i)
            printf(" %02X", dbg->scratch[offset + i]);

        for (size_t i = 0; i < Z80_MAX_INST_SIZE - inst.size; ++i)
            printf("   ");

        printf("   ");

        z80_print_inst(&inst, stdout);
        puts("");
        offset += inst.size;
    }
}

const struct cmd command_disassemble = {
    .type = CMD_TYPE_LEAF,
    .name = "disassemble",
    .help = "Disassemble a region of memory",
    {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "Address to start disassembling from."},
            {VALUE_TYPE_INT, "amount", "Number of bytes to disassemble."},
            {}
        },
        .payload = disassemble
    }}
};
