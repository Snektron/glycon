#include "commands/commands.h"
#include "debugger.h"

#include "common/glycon.h"

const struct cmd* commands[] = {
    &command_help,
    &command_quit,
    &command_memory,
    &command_connection,
    &command_ping,
    &command_flash,
    NULL
};

const struct cmd_option subcommand_write_opts[] = {
    {"repeat", 'r', VALUE_TYPE_INT, "amount", "Repeat the data the given amount of times before writing (default: 1)."},
    {"width", 'w', VALUE_TYPE_INT, "width", "Width in bytes of each individual data point. Values will be truncated to this size. (default 1)."},
    {}
};

const struct cmd_positional subcommand_write_pos[] = {
    {VALUE_TYPE_INT, "address", "The address to write to."},
    {VALUE_TYPE_INT, "value", "Values to write.", CMD_VARIADIC},
    {}
};

bool subcommand_write(struct debugger* dbg, const struct cmd_parse_result* args, uint16_t* address, size_t* size, uint8_t* buffer) {
    int64_t repeat = args->options[0].present ? args->options[0].value.as_int : 1;
    int64_t width = args->options[1].present ? args->options[1].value.as_int : 1;

    if (repeat < 1) {
        debugger_print_error(dbg, "Repeat %ld should be at least 1.", repeat);
        return true;
    } else if (width < 1 || width > 8) {
        debugger_print_error(dbg, "Width %ld outside of valid range [1, 8].", width);
        return true;
    }

    int64_t address_arg = args->positionals[0].as_int;
    if (address_arg < 0 || address_arg > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "Address %ld outside of valid range [0, %d).", address_arg, GLYCON_ADDRSPACE_SIZE);
        return true;
    }

    size_t i = 0;
    for (size_t j = 0; j < repeat; ++j) {
        for (size_t k = 1; k < args->positionals_len; ++k) {
            int64_t value = args->positionals[k].as_int;
            for (size_t l = 0; l < width; ++l) {
                uint8_t data = (value >> (l * 8)) & 0xFF;
                if (address_arg + i > GLYCON_ADDRSPACE_SIZE) {
                    debugger_print_error(dbg, "Write overflows address space.");
                    return true;
                }
                buffer[i++] = data;
            }
        }
    }

    *address = address_arg;
    *size = i;
    return false;
}
