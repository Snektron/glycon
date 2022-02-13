#ifndef _GLYDB_COMMAND_H
#define _GLYDB_COMMAND_H

#include "value.h"

#include <stdbool.h>
#include <stddef.h>

// This file implements the command handling system for the debugger. Commands are described
// using an array of structures, which are passed to a parsing function in order to parse them.

struct parser;

// This structure describes an option to a particular command.
struct cmd_option {
    // The name of the option in --long form.
    const char* name;
    // The name of the option in -short form.
    char shorthand;
    // The value type of this option. The value is supplied after the name. If this is
    // VALUE_TYPE_BOOL, the value is ommitted and simply set to `true` if this option is used.
    enum value_type value_type;
    // The name of the value.
    const char* value_name;
    // Help string describing the usage of this option.
    const char* help;
};

// Flags that may modify the behavior when a cmd_positional is parsed.
enum cmd_flag {
    // This positional argument is optional.
    CMD_OPTIONAL = 1 << 0,
    // This positional argument is variadic - it may take any number of values.
    CMD_VARIADIC = 2 << 0
};

// This structure describes a positional argument.
struct cmd_positional {
    // The value type that this positional argument accepts.
    enum value_type value_type;
    // The name of the value.
    const char* value_name;
    // Help string describing the usage of this parameter.
    const char* help;
    // Any flags that alter the behavior of this parameter.
    enum cmd_flag flags;
};

// Different types of command.
enum cmd_type {
    // This command is a directory - it hosts a number of subcommands.
    CMD_TYPE_DIRECTORY,
    // This command is a leaf - it may accept parameters and options.
    CMD_TYPE_LEAF
};

// Additional values for directory-type commands.
struct cmd_directory {
    // An array of subcommands in this directory. The last subcommand must be default-constructed.
    const struct cmd** subcommands;
};

// Additional values for leaf-type commands.
struct cmd_leaf {
    // Options that this command takes. The last option must be default-constructed.
    const struct cmd_option* options;
    // Positionals that this command takes. The last positional must be default-constructed.
    const struct cmd_positional* positionals;
    // User-defined payload for this leaf. This can be useful to tell the matched command apart.
    void* payload;
};

// A structure describing a particular command.
struct cmd {
    // The type of this command.
    enum cmd_type type;
    // The name that will be used to parse it.
    // Must be unique among commands in this directory.
    const char* name;
    // Help message describing the usage of this command.
    const char* help;

    // Any extra data associated with this command.
    union {
        struct cmd_directory directory;
        struct cmd_leaf leaf;
    };
};

// This structure represents a parsed option.
struct cmd_parsed_optional {
    // The value of this option. Type corresponds with the declared value type.
    union value_data value;
    // Whether this option was present at all.
    bool present;
};

// This structure represents a parsed command.
struct cmd_parse_result {
    // The most recently matched command.
    // May be empty if the input was empty, which is technically a valid command.
    const struct cmd* matched_command;

    // One for each of the matched command's options array.
    struct cmd_parsed_optional* options;

    // Positional arguments. Value type corresponds with the declared type.
    union value_data* positionals;
    // Number of positionals that were matched.
    size_t positionals_len;
};

// Parse a command according to the top-level-directory in `spec` (terminated by default-constructed cmd),
// and store the result in `result`. Returns `true` if parsing is successful, and `false` otherwise. If `true`,
// `result` is valid.
bool cmd_parse(struct cmd_parse_result* result, struct parser* p, const struct cmd* const* spec);

// Frees any data owned by `result`.
void cmd_parse_result_deinit(struct cmd_parse_result* result);

// Return the command from `spec` that best matched a name given by `command`. Returns `NULL` if there was
// not a good match.
const struct cmd* cmd_match_command(const struct cmd* const* spec, size_t len, const char command[len]);

// Return wether this option is valid. Default-constructed options return invalid.
inline bool cmd_option_is_valid(const struct cmd_option* opt) {
    return opt->name || opt->shorthand;
}

// Return whether this positional is valid. Default-constructed positionals return invalid.
inline bool cmd_positional_is_valid(const struct cmd_positional* pos) {
    return pos->value_name;
}

#endif
