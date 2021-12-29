#include "command/command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static void print_command(struct cmd_parser* cmdp) {
    struct parser p = cmdp->p;
    p.offset = 0;
    bool first = true;

    const struct cmd* cmd = cmdp->spec;
    while (p.offset <= cmdp->command_offset) {
        parser_skip_ws(&p);
        const char* command = parser_remaining(&p);
        size_t command_len = parser_eat_word(&p);
        assert(command_len != 0);

        if (first) {
            first = false;
        } else {
            printf(" ");
        }

        while (cmd->type != CMD_TYPE_END && strncmp(command, cmd->name, command_len) != 0) {
            ++cmd;
            continue;
        }

        if (cmd->type == CMD_TYPE_END) {
            // No match: just print the word
            printf("%.*s", (int) command_len, command);
        } else {
            // Print the full name for clarity.
            printf("%s", cmd->name);
        }

        if (cmd->type == CMD_TYPE_DIRECTORY)
            cmd = cmd->directory.subcommands;
    }
}

static void report_invalid_command(struct cmd_parser* cmdp) {
    printf("error: invalid command '");
    print_command(cmdp);
    puts("'");
}

static void report_unexpected_character(struct cmd_parser* cmdp) {
    int c = parser_peek(&cmdp->p);
    if (c < 0) {
        printf("error: unexpected end of command\n");
    } else if (isprint(c)) {
        printf("error: unexpected character %c\n", c);
    } else {
        printf("error: unexpected character \\x%02X\n", c);
    }
}

static void report_missing_argument(struct cmd_parser* cmdp, const struct cmd_option* opt, bool shorthand) {
    printf("error: ");
    print_command(cmdp);
    printf(": missing argument <%s> to option ", opt->value_name);

    if (shorthand) {
        printf("-%c\n", opt->shorthand);
    } else {
        printf("--%s\n", opt->name);
    }
}

static void report_invalid_long_option(struct cmd_parser* cmdp, size_t len, const char opt[len]) {
    printf("error: ");
    print_command(cmdp);
    printf(": invalid option --%.*s\n", (int) len, opt);
}

static void report_invalid_short_option(struct cmd_parser* cmdp, char opt) {
    printf("error: ");
    print_command(cmdp);
    printf(": invalid option -%c\n", opt);
}

static void report_duplicate_option(struct cmd_parser* cmdp, const struct cmd_option* opt) {
    printf("error: ");
    print_command(cmdp);
    if (opt->shorthand && opt->name) {
        printf(": duplicate option -%c/--%s\n", opt->shorthand, opt->name);
    } else if (opt->shorthand) {
        printf(": duplicate option -%c\n", opt->shorthand);
    } else {
        printf(": duplicate option --%s\n", opt->name);
    }
}

static void report_incorrect_positionals(struct cmd_parser* cmdp) {
    printf("error: ");
    print_command(cmdp);

    if (cmdp->positionals_len > cmdp->matched_command->leaf.positionals) {
        printf(": too many positional arguments\n");
    } else {
        printf(": not enough positional arguments\n");
    }
}

static bool is_flag(const char text[]) {
    return text[0] == '-';
}

static bool parse_long_option(struct cmd_parser* cmdp, const struct cmd_option* options) {
    struct parser* p = &cmdp->p;
    // At this point, the leading -- is already parsed.
    const char* option = parser_remaining(p);
    size_t option_len = parser_eat_word(p);
    if (option_len == 0) {
        // Note: Also matches '--'.
        report_unexpected_character(cmdp);
        return false;
    }

    for (size_t i = 0; options && (options[i].name || options[i].shorthand); ++i) {
        const struct cmd_option* opt = &options[i];
        if (strncmp(opt->name, option, option_len) != 0)
            continue;

        if (cmdp->options[i]) {
            report_duplicate_option(cmdp, opt);
            return false;
        }

        if (!opt->value_name) {
            cmdp->options[i] = strdup("");
            return true;
        }

        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            report_missing_argument(cmdp, opt, false);
            return true;
        }

        // TODO: Integrate with expression parsing.
        const char* option_arg = parser_remaining(p);
        size_t option_arg_len = parser_eat_word(p);

        if (option_arg_len == 0) {
            report_unexpected_character(cmdp);
            return false;
        }

        cmdp->options[i] = strndup(option_arg, option_arg_len);
        return true;
    }

    report_invalid_long_option(cmdp, option_len, option);
    return false;
}

static bool parse_short_options(struct cmd_parser* cmdp, const struct cmd_option* options) {
    struct parser* p = &cmdp->p;
    // At this point, the leading - is already parsed.
    // Note: We might support multiple forms of short flags depending on the flag's type:
    // - -x arg: x with argument 'arg'
    // - -xarg: x with argument 'arg'
    // - -xy arg: x as flag, y with argument 'arg'
    // - -xyarg: x as flag, y with argument 'arg'
    while (true) {
    next_option:
        if (parser_is_at_end(p) || parser_test_ws(p))
            return true;

        int option = parser_peek(p);
        for (size_t i = 0; options && (options[i].name || options[i].shorthand); ++i) {
            const struct cmd_option* opt = &options[i];
            if (opt->shorthand != option)
                continue;

            ++p->offset;

            if (cmdp->options[i]) {
                report_duplicate_option(cmdp, opt);
                return false;
            }

            if (!opt->value_name) {
                cmdp->options[i] = strdup("");
                goto next_option;
            }

            parser_skip_ws(p);
            if (parser_is_at_end(p)) {
                report_missing_argument(cmdp, opt, true);
                return false;
            }

            // TODO: Integrate with expression parsing.
            const char* option_arg = parser_remaining(p);
            size_t option_arg_len = parser_eat_word(p);

            if (option_arg_len == 0) {
                report_unexpected_character(cmdp);
                return false;
            }

            cmdp->options[i] = strndup(option_arg, option_arg_len);
            return true;
        }

        report_invalid_short_option(cmdp, option);
        return false;
    }
}

static bool parse_leaf(struct cmd_parser* cmdp) {
    const struct cmd* cmd = cmdp->matched_command;
    size_t num_optionals = 0;
    {
        const struct cmd_option* opt = cmd->leaf.options;
        while (opt && (opt->name || opt->shorthand)) {
            ++num_optionals;
            ++opt;
        }
    }

    cmdp->options = calloc(num_optionals, sizeof(const char*));

    struct parser* p = &cmdp->p;
    while (true) {
        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            break;
        } else if (parser_peek(p) != '-') {
            // Parse a positional argument.
            // TODO: Integrate with expression parsing.
            const char* positional = parser_remaining(p);
            size_t positional_len = parser_eat_word(p);
            if (positional_len == 0) {
                report_unexpected_character(cmdp);
                return false;
            }

            // TODO: More efficient realloc
            size_t i = cmdp->positionals_len++;
            cmdp->positionals = realloc(cmdp->positionals, cmdp->positionals_len);
            cmdp->positionals[i] = strndup(positional, positional_len);
            continue;
        }

        ++p->offset;
        if (parser_peek(p) == '-') {
            // Long argument
            ++p->offset;
            if (!parse_long_option(cmdp, cmd->leaf.options))
                return false;
        } else {
            // Short argument
            if (!parse_short_options(cmdp, cmd->leaf.options))
                return false;
        }
    }

    if (cmd->leaf.positionals != CMD_VARIADIC && cmd->leaf.positionals != cmdp->positionals_len) {
        report_incorrect_positionals(cmdp);
        return false;
    }

    return true;
}

static bool parse_cmd(struct cmd_parser* cmdp, const struct cmd* spec) {
    struct parser* p = &cmdp->p;
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        // Matched no command. In this case we either match the directory or
        // the empty input. In either case, cmdp->matched_command is already
        // correct.
        return true;
    }

    cmdp->command_offset = p->offset;
    const char* command = parser_remaining(p);
    size_t command_len = parser_eat_word(p);

    if (command_len == 0) {
        report_unexpected_character(cmdp);
        return false;
    } else if (is_flag(command)) {
        report_invalid_command(cmdp);
        return false;
    }

    const struct cmd* cmd = spec;
    while (cmd->type != CMD_TYPE_END) {
        const char* name = cmd->name;
        if (strncmp(command, name, command_len) != 0) {
            ++cmd;
            continue;
        }

        // Matched a (sub)command.
        cmdp->matched_command = cmd;
        switch (cmd->type) {
            case CMD_TYPE_END:
                assert(false);
            case CMD_TYPE_DIRECTORY:
                return parse_cmd(cmdp, cmd->directory.subcommands);
            case CMD_TYPE_LEAF:
                return parse_leaf(cmdp);
        }
    }

    // No commands matched.
    report_invalid_command(cmdp);
    return false;
}

void cmd_parser_init(struct cmd_parser* cmdp, const struct cmd* spec, size_t len, const char line[len]) {
    parser_init(&cmdp->p, len, line);
    cmdp->spec = spec;
    cmdp->matched_command = NULL;
    cmdp->command_offset = 0;
    cmdp->options = NULL;
    cmdp->positionals = NULL;
    cmdp->positionals_len = 0;
}

void cmd_parser_deinit(struct cmd_parser* cmdp) {
    if (cmdp->matched_command && cmdp->matched_command->type == CMD_TYPE_LEAF) {
        const struct cmd_option* options = cmdp->matched_command->leaf.options;
        for (size_t i = 0; options && (options[i].name || options[i].shorthand); ++i) {
            free(cmdp->options[i]);
        }
    }

    for (size_t i = 0; i < cmdp->positionals_len; ++i) {
        free(cmdp->positionals[i]);
    }

    free(cmdp->options);
    free(cmdp->positionals);
}

bool cmd_parse(struct cmd_parser* cmdp) {
    return parse_cmd(cmdp, cmdp->spec);
}

