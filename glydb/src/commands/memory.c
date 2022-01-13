#include "commands/commands.h"
#include "debugger.h"
#include "glycon.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    int64_t repeat = args->options[0].present ? args->options[0].value.as_int : 1;
    int64_t width = args->options[1].present ? args->options[1].value.as_int : 1;

    if (repeat < 1) {
        debugger_print_error(dbg, "Repeat %ld should be at least 1.", repeat);
        return;
    } else if (width < 1 || width > 8) {
        debugger_print_error(dbg, "Width %ld outside of valid range [1, 8].", width);
        return;
    }

    uint16_t address = args->positionals[0].as_int;
    size_t i = 0;
    for (size_t j = 0; j < repeat; ++j) {
        for (size_t k = 1; k < args->positionals_len; ++k) {
            int64_t value = args->positionals[k].as_int;
            for (size_t l = 0; l < width; ++l) {
                uint8_t data = (value >> (l * 8)) & 0xFF;
                // Note: this check also prevents overflowing the scratch buffer.
                if (((uint16_t)(address + i) & GLYCON_RAM_MASK) == 0) {
                    debugger_print_error(dbg, "`memory write` cannot write to flash.");
                    return;
                }
                dbg->scratch[i++] = data;
            }
        }
    }

    debugger_write_memory(dbg, address, i, dbg->scratch);
}

static void memory_read(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    int64_t amt = args->positionals_len > 1 ? args->positionals[1].as_int : 1;
    if (amt < 1 || amt > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "amount %ld outside valid range [1, %d]", amt, GLYCON_ADDRSPACE_SIZE);
        return;
    }

    if (debugger_read_memory(dbg, address, amt, dbg->scratch))
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
        .options = (struct cmd_option[]){
            {"repeat", 'r', VALUE_TYPE_INT, "amount", "Repeat the data the given amount of times before writing (default: 1)."},
            {"width", 'w', VALUE_TYPE_INT, "width", "Width in bytes of each individual data point. Values will be truncated to this size. (default 1)."},
            {}
        },
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "value", "Values to write.", CMD_VARIADIC},
            {}
        },
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
    .help = "Manipulate target memory",
    {.directory = {memory_commands}}
};
