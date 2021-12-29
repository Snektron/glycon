#ifndef _GLYDB_COMMAND_COMMAND_H
#define _GLYDB_COMMAND_COMMAND_H

#include <stdbool.h>
#include <stddef.h>

struct interpreter;

typedef void (*cmd_handler_t)(struct interpreter* interp);

struct cmd_option {
    const char* name;
    char shorthand;
    const char* help;
    bool takes_argument;
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
    bool has_freeform_args;
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

void cmd_parse(const struct cmd* spec, size_t len, const char line[len]);

#endif
