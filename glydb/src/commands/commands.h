#ifndef _GLYDB_COMMANDS_COMMANDS_H
#define _GLYDB_COMMANDS_COMMANDS_H

#include "command.h"

#include <stdint.h>
#include <stddef.h>

struct debugger;

typedef void (*command_handler_t)(struct debugger* dbg, const struct cmd_parse_result* args);

extern const struct cmd* commands[];

extern const struct cmd command_help;
extern const struct cmd command_quit;
extern const struct cmd command_memory;
extern const struct cmd command_connection;
extern const struct cmd command_ping;
extern const struct cmd command_flash;

// Shared functionality between memory/flash write commands

extern const struct cmd_option subcommand_write_opts[];
extern const struct cmd_positional subcommand_write_pos[];

// Handle the common write subcommand. This function fills `buffer` with data (which should be large enough
// to contain the the write, which is bounded by the address space size), and returns `false` is no error
// occurred.
// If this function returns `true`, an error occured, and the write should not be performed.
// In this case an error message has already been printed.
bool subcommand_write(struct debugger* dbg, const struct cmd_parse_result* args, uint16_t* address, size_t* size, uint8_t* buffer);

#endif
