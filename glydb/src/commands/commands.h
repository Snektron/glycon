#ifndef _GLYDB_COMMANDS_COMMANDS_H
#define _GLYDB_COMMANDS_COMMANDS_H

#include "command.h"

struct debugger;

typedef void (*command_handler_t)(struct debugger* dbg, const struct cmd_parse_result* args);

extern const struct cmd* commands[];

extern const struct cmd command_help;
extern const struct cmd command_quit;
extern const struct cmd command_memory;
extern const struct cmd command_connection;
extern const struct cmd command_ping;
extern const struct cmd command_flash;

#endif
