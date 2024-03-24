#include "commands/commands.h"
#include "debugger.h"
#include "connection.h"

#include "common/glycon.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

const struct cmd* commands[] = {
    &command_help,
    &command_quit,
    &command_memory,
    &command_connection,
    &command_ping,
    &command_flash,
    &command_disassemble,
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

bool subcommand_write(struct debugger* dbg, const struct cmd_parse_result* args, struct debugger_write_op* op, uint8_t* buffer) {
    int64_t repeat = args->options[0].present ? args->options[0].value.as_int : 1;
    int64_t width = args->options[1].present ? args->options[1].value.as_int : 1;

    if (repeat < 1) {
        debugger_print_error(dbg, "Repeat %ld should be at least 1.", repeat);
        return true;
    } else if (width < 1 || width > 8) {
        debugger_print_error(dbg, "Width %ld outside of valid range [1, 8].", width);
        return true;
    }

    int64_t address = args->positionals[0].as_int;
    if (address < 0 || address > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "Address %ld outside of valid range [0, %d).", address, GLYCON_ADDRSPACE_SIZE);
        return true;
    }

    size_t i = 0;
    for (size_t j = 0; j < repeat; ++j) {
        for (size_t k = 1; k < args->positionals_len; ++k) {
            int64_t value = args->positionals[k].as_int;
            for (size_t l = 0; l < width; ++l) {
                uint8_t data = (value >> (l * 8)) & 0xFF;
                if (address + i > GLYCON_ADDRSPACE_SIZE) {
                    debugger_print_error(dbg, "Write overflows address space.");
                    return true;
                }
                buffer[i++] = data;
            }
        }
    }

    op->address = address;
    op->len = i;
    return false;
}

const struct cmd_option subcommand_load_opts[] = {
    {"type", 't', VALUE_TYPE_STR, "file type", "Override file type to either ihx or bin (default: infer from filename)."},
    {}
};

const struct cmd_positional subcommand_load_pos[] = {
    {VALUE_TYPE_STR, "filename", "Path specifying the file to load."},
    {VALUE_TYPE_INT, "relocation", "Relocation address, which will be added to each data base address. The base address for `bin` files is 0.", CMD_OPTIONAL},
    {}
};

bool subcommand_load(struct debugger* dbg, const struct cmd_parse_result* args, struct debugger_write_op** ops, uint8_t* buffer) {
    const char* path = args->positionals[0].as_str;
    int64_t relocation = args->positionals_len > 1 ? args->positionals[1].as_int : 0;
    if (relocation < 0 || relocation > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "Relocation address %ld outside of valid range [0, %d).", relocation, GLYCON_ADDRSPACE_SIZE);
        return true;
    }

    struct debugger_load_file_options opts = {
        .path = path,
        .ext_override = args->options[0].present ? args->options[0].value.as_str : NULL,
        .relocation = relocation
    };

    return debugger_load_file(dbg, &opts, ops, buffer);
}

bool subcommand_open(struct debugger* dbg, const char* port) {
    if (conn_is_open(&dbg->conn)) {
        debugger_print_error(dbg, "A connection is already open. Close it first with `connection close`.");
        return true;
    } else if (!conn_open_serial(&dbg->conn, port)) {
        debugger_print_error(dbg, "Failed to open serial port '%s': %s.", port, strerror(errno));
        return true;
    }

    return false;
}
