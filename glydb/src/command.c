#include "command.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static void report_invalid_command(size_t len, const char text[len]) {
    printf("error: invalid command '%.*s'\n", (int) len, text);
}

static void report_unexpected_character(int c) {
    if (c < 0) {
        printf("error: unexpected end of command.\n");
    } else if (isprint(c)) {
        printf("error: unexpected character %c.\n", c);
    } else {
        printf("error: unexpected character \\x%02X.\n", c);
    }
}

static void report_missing_argument(const struct cmd_option* opt, bool shorthand) {
    printf("error: missing argument <%s> to option ", opt->value_name);
    if (shorthand) {
        printf("-%c.\n", opt->shorthand);
    } else {
        printf("--%s.\n", opt->name);
    }
}

static void report_invalid_long_option(size_t len, const char opt[len]) {
    printf("error: invalid option --%.*s.\n", (int) len, opt);
}

static void report_invalid_short_option(char opt) {
    printf("error: invalid option -%c.\n", opt);
}

static void report_duplicate_option(const struct cmd_option* opt) {
    printf("error: duplicate option ");
    if (opt->shorthand && opt->name) {
        printf("-%c/--%s.\n", opt->shorthand, opt->name);
    } else if (opt->shorthand) {
        printf("-%c.\n", opt->shorthand);
    } else {
        printf("--%s.\n", opt->name);
    }
}

static void report_missing_positional(const struct cmd* cmd, size_t i) {
    printf("error: missing required positional argument <%s>.\n", cmd->leaf.positionals[i].value_name);
}

static void report_superficial_positional(size_t len, const char pos[len]) {
    printf("error: superficial positional argument '%.*s'.\n", (int) len, pos);
}

static bool is_flag(const char text[]) {
    return text[0] == '-';
}

static bool parse_long_option(struct cmd_parse_result* result, struct parser* p, const struct cmd_option* options) {
    // At this point, the leading -- is already parsed.
    const char* option = parser_remaining(p);
    size_t option_len = parser_eat_word(p);
    if (option_len == 0) {
        // Note: Also matches '--'.
        report_unexpected_character(parser_peek(p));
        return false;
    }

    for (size_t i = 0; options && cmd_option_is_valid(&options[i]); ++i) {
        const struct cmd_option* opt = &options[i];
        if (strncmp(opt->name, option, option_len) != 0)
            continue;

        if (result->options[i]) {
            report_duplicate_option(opt);
            return false;
        }

        if (!opt->value_name) {
            result->options[i] = strdup("");
            return true;
        }

        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            report_missing_argument(opt, false);
            return true;
        }

        // TODO: Integrate with expression parsing.
        const char* option_arg = parser_remaining(p);
        size_t option_arg_len = parser_eat_word(p);

        if (option_arg_len == 0) {
            report_unexpected_character(parser_peek(p));
            return false;
        }

        result->options[i] = strndup(option_arg, option_arg_len);
        return true;
    }

    report_invalid_long_option(option_len, option);
    return false;
}

static bool parse_short_options(struct cmd_parse_result* result, struct parser* p, const struct cmd_option* options) {
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

            if (result->options[i]) {
                report_duplicate_option(opt);
                return false;
            }

            ++p->offset;

            if (!opt->value_name) {
                result->options[i] = strdup("");
                goto next_option;
            }

            parser_skip_ws(p);
            if (parser_is_at_end(p)) {
                report_missing_argument(opt, true);
                return false;
            }

            // TODO: Integrate with expression parsing.
            const char* option_arg = parser_remaining(p);
            size_t option_arg_len = parser_eat_word(p);

            if (option_arg_len == 0) {
                report_unexpected_character(parser_peek(p));
                return false;
            }

            result->options[i] = strndup(option_arg, option_arg_len);
            return true;
        }

        report_invalid_short_option(option);
        return false;
    }
}

static bool parse_leaf(struct cmd_parse_result* result, struct parser* p) {
    const struct cmd* cmd = result->matched_command;
    size_t num_optionals = 0;
    {
        const struct cmd_option* opt = cmd->leaf.options;
        while (opt && (opt->name || opt->shorthand)) {
            ++num_optionals;
            ++opt;
        }
    }
    size_t min_positionals = 0;
    size_t max_positionals = 0;
    bool variadic = false;
    {
        const struct cmd_positional* pos = cmd->leaf.positionals;
        bool seen_optional = false;
        while (pos && cmd_positional_is_valid(pos)) {
            assert(!variadic); // Must only occur on the last positional
            ++max_positionals;
            if (pos->flags & CMD_OPTIONAL) {
                seen_optional = true;
            } else {
                assert(!seen_optional);
                ++min_positionals;
            }

            if (pos->flags & CMD_VARIADIC)
                variadic = true;

            ++pos;
        }
    }

    result->options = calloc(num_optionals, sizeof(const char*));
    result->positionals = calloc(min_positionals, sizeof(const char*));

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
                report_unexpected_character(parser_peek(p));
                return false;
            }

            size_t i = result->positionals_len++;
            if (i == max_positionals && !variadic) {
                report_superficial_positional(positional_len, positional);
                return false;
            }

            if (result->positionals_len >= min_positionals) {
                // TODO: More efficient realloc
                result->positionals = realloc(result->positionals, result->positionals_len);
            }
            result->positionals[i] = strndup(positional, positional_len);
            continue;
        }

        ++p->offset;
        if (parser_peek(p) == '-') {
            // Long argument
            ++p->offset;
            if (!parse_long_option(result, p, cmd->leaf.options))
                return false;
        } else {
            // Short argument
            if (!parse_short_options(result, p, cmd->leaf.options))
                return false;
        }
    }

    if (result->positionals_len < min_positionals) {
        report_missing_positional(result->matched_command, result->positionals_len);
        return false;
    }

    return true;
}

static bool parse_cmd(struct cmd_parse_result* result, struct parser* p, const struct cmd* const* spec) {
    while (true) {
        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            // Matched no command. In this case we either match the directory or
            // the empty input. In either case, result->matched_command is already
            // correct.
            return true;
        }

        const char* command = parser_remaining(p);
        size_t command_len = parser_eat_word(p);

        if (command_len == 0) {
            report_unexpected_character(parser_peek(p));
            return false;
        } else if (is_flag(command)) {
            report_invalid_command(command_len, command);
            return false;
        }

        result->matched_command = cmd_match_command(spec, command_len, command);
        if (!result->matched_command) {
            report_invalid_command(command_len, command);
            return false;
        }

        switch (result->matched_command->type) {
            case CMD_TYPE_LEAF:
                return parse_leaf(result, p);
            case CMD_TYPE_DIRECTORY:
                spec = result->matched_command->directory.subcommands;
                break;
        }
    }
}

bool cmd_parse(struct cmd_parse_result* result, struct parser* p, const struct cmd* const* spec) {
    result->matched_command = NULL;
    result->options = NULL;
    result->positionals = NULL;
    result->positionals_len = 0;
    return parse_cmd(result, p, spec);
}

void cmd_parse_result_deinit(struct cmd_parse_result* result) {
    if (result->matched_command && result->matched_command->type == CMD_TYPE_LEAF) {
        const struct cmd_option* options = result->matched_command->leaf.options;
        for (size_t i = 0; options && cmd_option_is_valid(&options[i]); ++i) {
            free(result->options[i]);
        }
    }

    for (size_t i = 0; i < result->positionals_len; ++i) {
        free(result->positionals[i]);
    }

    free(result->options);
    free(result->positionals);
}

const struct cmd* cmd_match_command(const struct cmd* const* spec, size_t len, const char command[len]) {
    if (!spec)
        return NULL;

    for (size_t i = 0; spec[i]; ++i) {
        const struct cmd* cmd = spec[i];
        if (strncmp(command, cmd->name, len) == 0) {
            return cmd;
        }
    }

    return NULL;
}
