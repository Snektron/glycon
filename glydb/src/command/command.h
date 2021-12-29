#ifndef _GLYDB_COMMAND_COMMAND_H
#define _GLYDB_COMMAND_COMMAND_H

#include <stdbool.h>
#include <stddef.h>

#include "command/parser.h"

#define CMD_VARIADIC (-1)

struct interpreter;

typedef void (*cmd_handler_t)(struct interpreter* interp);

struct cmd_option {
    const char* name;
    char shorthand;
    const char* help;
    const char* value_name; // If NULL takes no value
};

enum cmd_type {
    CMD_TYPE_END = 0,
    CMD_TYPE_DIRECTORY,
    CMD_TYPE_LEAF
};

struct cmd_directory {
    const struct cmd* subcommands;
};

struct cmd_leaf {
    const struct cmd_option* options;
    int positionals; // Allows CMD_VARIADIC
    cmd_handler_t handler;
};

struct cmd {
    enum cmd_type type;
    const char* name;
    const char* help;

    union {
        struct cmd_directory directory;
        struct cmd_leaf leaf;
    };
};

struct cmd_parser {
    const struct cmd* spec;
    struct parser p;

    // The most recently matched command.
    // May be empty if the input was empty, which is technically a valid command.
    const struct cmd* matched_command;

    // Start offset of the most recently matched command (for error reporting).
    // This includes invalid commands, in which case this offset will not actually
    // correspond with that of `matched_command`.
    size_t command_offset;
};

void cmd_parser_init(struct cmd_parser* cmdp, const struct cmd* spec, size_t len, const char line[len]);

bool cmd_parse(struct cmd_parser* cmdp);

#endif
