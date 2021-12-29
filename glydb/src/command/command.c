#include "command/command.h"
#include "command/parser.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

static bool is_flag(const char text[]) {
    return text[0] == '-';
}

static void parse_long_option(const struct cmd_option* options, struct parser* p) {
    // At this point, the leading -- is already parsed.
    const char* option = parser_remaining(p);
    size_t option_len = parser_eat_word(p);
    if (option_len == 0) {
        // TODO: Proper error handling.
        // Note: Also matches '--'.
        printf("Unexpected characters in command %.*s\n", (int) p->length, p->input);
        return;
    }

    const struct cmd_option* opt = options;
    while (options && (opt->name || opt->shorthand)) {
        if (strncmp(opt->name, option, option_len) != 0) {
            ++opt;
            continue;
        }

        if (!opt->value_name) {
            // TODO: Add to parsed options list.
            printf("Matched option --%s\n", opt->name);
            return;
        }

        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            // TODO: Proper error handling
            printf("Missing argument <%s> for option --%s\n", opt->value_name, opt->name);
            return;
        }

        const char* option_arg = parser_remaining(p);
        size_t option_arg_len = parser_eat_word(p);

        if (option_arg_len == 0) {
            // TODO: Proper error handling
            printf("Unexpected characters in command %.*s\n", (int) p->length, p->input);
            return;
        }

        printf("Matched option --%s with argument %.*s\n", opt->name, (int) option_arg_len, option_arg);
        return;
    }

    printf("Invalid option --%.*s\n", (int) option_len, option);
}

static void parse_short_options(const struct cmd_option* options, struct parser* p) {
    // At this point, the leading - is already parsed.
    // Note: We might support multiple forms of short flags depending on the flag's type:
    // - -x arg: x with argument 'arg'
    // - -xarg: x with argument 'arg'
    // - -xy arg: x as flag, y with argument 'arg'
    // - -xyarg: x as flag, y with argument 'arg'
    while (true) {
    next_option:
        if (parser_is_at_end(p) || parser_test_ws(p))
            break;

        int option = parser_peek(p);
        const struct cmd_option* opt = options;
        while (options && (opt->name || opt->shorthand)) {
            if (opt->shorthand != option) {
                ++opt;
                continue;
            }
            ++p->offset;

            if (!opt->value_name) {
                printf("Matched option '-%c'\n", option);
                goto next_option;
            }

            parser_skip_ws(p);
            if (parser_is_at_end(p)) {
                printf("Missing argument <%s> for option -%c\n", opt->value_name, option);
                return;
            }

            const char* option_arg = parser_remaining(p);
            size_t option_arg_len = parser_eat_word(p);

            if (option_arg_len == 0) {
                // TODO: Proper error handling
                printf("Unexpected characters in command %.*s\n", (int) p->length, p->input);
                return;
            }

            printf("Matched option -%c with argument %.*s\n", option, (int) option_arg_len, option_arg);
            return;
        }

        printf("Invalid short option '-%c'\n", option);
        return;
    }
}

static void parse_leaf(const struct cmd* cmd, struct parser* p) {
    while (true) {
        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            break;
        } else if (parser_peek(p) != '-') {
            // Parse a positional argument.
            // TODO: Integrate with expression parsing.
            // TODO: Apprent to some positional arguments lists.
            // For that we probably also want to keep some buffer with a longer lifetime.
            const char* positional = parser_remaining(p);
            size_t positional_len = parser_eat_word(p);
            if (positional_len == 0) {
                // TODO: Proper error handling.
                printf("Unexpected characters in command %.*s\n", (int) p->length, p->input);
                return;
            }

            printf("Matched positional argument '%.*s'\n", (int) positional_len, positional);
            continue;
        }

        ++p->offset;
        if (parser_peek(p) == '-') {
            // Long argument
            ++p->offset;
            parse_long_option(cmd->leaf.options, p);
        } else {
            // Short argument
            parse_short_options(cmd->leaf.options, p);
        }
    }

    printf("Matched command '%s'\n", cmd->name);
}

static void parse_cmd(const struct cmd* spec, struct parser* p) {
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        printf("Expected a (sub)command in command '%.*s'\n", (int) p->length, p->input);
        return;
    }

    const char* command = parser_remaining(p);
    size_t command_len = parser_eat_word(p);

    if (command_len == 0) {
        // TODO: Proper error handling.
        printf("Unexpected characters in command '%.*s'\n", (int) p->length, p->input);
        return;
    } else if (is_flag(command)) {
        // TODO: Proper error handling.
        printf("Unexpected option %.*s in command '%.*s'\n", (int) command_len, command, (int) p->length, p->input);
        return;
    }

    const struct cmd* cmd = spec;
    while (cmd->type != CMD_TYPE_END) {
        const char* name = cmd->name;
        if (strncmp(command, name, command_len) != 0) {
            ++cmd;
            continue;
        }

        // Matched a (sub)command.
        switch (cmd->type) {
            case CMD_TYPE_END:
                assert(false);
            case CMD_TYPE_DIRECTORY:
                parse_cmd(cmd->directory.subcommands, p);
                return;
            case CMD_TYPE_LEAF:
                parse_leaf(cmd, p);
                return;
        }
    }

    // No commands matched.
    // TODO: Proper error handling.
    printf("Invalid command '%.*s'\n", (int) command_len, command);
}

void cmd_parse(const struct cmd* spec, size_t len, const char line[len]) {
    struct parser p;
    parser_init(&p, len, line);
    parse_cmd(spec, &p);
}

