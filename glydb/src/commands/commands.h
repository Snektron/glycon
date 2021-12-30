#ifndef _GLYDB_COMMANDS_COMMANDS_H
#define _GLYDB_COMMANDS_COMMANDS_H

#include "command.h"

struct interpreter;

typedef void (*command_handler_t)(struct interpreter* interp, size_t len, const char* const positionals[len]);

extern const struct cmd* commands[];

extern const struct cmd command_help;
extern const struct cmd command_quit;
extern const struct cmd command_memory;
extern const struct cmd command_connection;

#endif
