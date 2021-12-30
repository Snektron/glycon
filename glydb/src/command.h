#ifndef _GLYDB_COMMAND_H
#define _GLYDB_COMMAND_H

#include <stdbool.h>
#include <stddef.h>

#include "parser.h"

struct interpreter;

struct cmd_option {
    const char* name;
    char shorthand;
    const char* help;
    const char* value_name; // If NULL takes no value
};

enum cmd_flag {
    CMD_OPTIONAL = 1 << 0,
    CMD_VARIADIC = 2 << 0
};

struct cmd_positional {
    const char* value_name;
    const char* help;
    enum cmd_flag flags;
};

enum cmd_type {
    CMD_TYPE_DIRECTORY,
    CMD_TYPE_LEAF
};

struct cmd_directory {
    const struct cmd** subcommands;
};

struct cmd_leaf {
    const struct cmd_option* options;
    const struct cmd_positional* positionals;
    void* payload;
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
    struct parser p;
    const struct cmd* const* spec;

    // The most recently matched command.
    // May be empty if the input was empty, which is technically a valid command.
    const struct cmd* matched_command;

    // Start offset of the most recently matched command (for error reporting).
    // This includes invalid commands, in which case this offset will not actually
    // correspond with that of `matched_command`.
    size_t command_offset;

    // One for each of the matched command's options array.
    // For a flag without value, the option is set to "" if its present.
    char** options;
    char** positionals;
    size_t positionals_len;
};

void cmd_parser_init(struct cmd_parser* cmdp, const struct cmd* const* spec, size_t len, const char line[len]);

void cmd_parser_deinit(struct cmd_parser* cmdp);

bool cmd_parse(struct cmd_parser* cmdp);

const struct cmd* cmd_match_command(const struct cmd* const* spec, size_t len, const char command[len]);

inline bool cmd_option_is_valid(const struct cmd_option* opt) {
    return opt->name || opt->shorthand;
}

inline bool cmd_positional_is_valid(const struct cmd_positional* pos) {
    return pos->value_name;
}

#endif
