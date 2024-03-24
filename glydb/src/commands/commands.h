#ifndef _GLYDB_COMMANDS_COMMANDS_H
#define _GLYDB_COMMANDS_COMMANDS_H

#include "common/glycon.h"

#include "command.h"

#include <stdint.h>
#include <stddef.h>

struct debugger;
struct debugger_write_op;

// The type of a command's payload - a handler function.
typedef void (*command_handler_t)(struct debugger* dbg, const struct cmd_parse_result* args);

// All commands that the debugger recognizes.
extern const struct cmd* commands[];

// Form definitions of commands.
extern const struct cmd command_help;
extern const struct cmd command_quit;
extern const struct cmd command_memory;
extern const struct cmd command_connection;
extern const struct cmd command_ping;
extern const struct cmd command_flash;
extern const struct cmd command_disassemble;

// A structure describing the target location of some amount of bytes that needs
// to be written. Bytes themselves are stored externally.
struct write_operation {
    gly_addr_t address;
    size_t len;
};

// Shared functionality between memory/flash write commands.

// Shared write-style command options.
extern const struct cmd_option subcommand_write_opts[];
// Shared write-style command positionals.
extern const struct cmd_positional subcommand_write_pos[];

// Handle the common write subcommand. This function fills `buffer` with data (which should be large enough
// to contain the the write, which is bounded by the address space size), and returns `false` is no error
// occurred.
// If this function returns `true`, an error occured, and the write should not be performed.
// In this case an error message has already been printed.
bool subcommand_write(struct debugger* dbg, const struct cmd_parse_result* args, struct debugger_write_op* op, uint8_t* buffer);

// Shared functionality between memory/flash load commands.

// Shared load-style command options.
extern const struct cmd_option subcommand_load_opts[];
// Shared load-style command positionals.
extern const struct cmd_positional subcommand_load_pos[];

// Handle the common load subcommand. This function loads a file from disk, and returns a number of buffers that need
// to be written.
// Note that a file may return multiple disjoint regions that need to be written to the device. For this reason
// this function accepts a pointer to a buffer that will be allocated with an array of `write` operations, each
// containing some number of bytes and a base address. The actual data for these operations will be written to `buffer`
// in a packed sequence, in the same order as appears in `areas`.
// Returns `true` if an error occurred, or `false` on success. In the former case, an error message
// is already printed.
bool subcommand_load(struct debugger* dbg, const struct cmd_parse_result* args, struct debugger_write_op** ops, uint8_t* buffer);

// Handle the common `open` command. This attempts to open a connection to `port`, and prints
// an error message on failure. Returns `true` if an error occurred, or `false` on success.
bool subcommand_open(struct debugger* dbg, const char* port);

#endif
