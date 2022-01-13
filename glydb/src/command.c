#include "command.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static void report_invalid_command(size_t len, const char text[len]) {
    printf("error: Invalid command '%.*s'.\n", (int) len, text);
}

static void report_unexpected_character(struct parser* p) {
    int c = parser_peek(p);
    if (c < 0) {
        printf("error: Unexpected end of command.\n");
    } else if (isprint(c)) {
        printf("error: Unexpected character %c.\n", c);
    } else {
        printf("error: Unexpected character \\x%02X.\n", c);
    }
}

static void report_missing_argument(const struct cmd_option* opt, bool shorthand) {
    printf("error: Missing argument <%s> to option ", opt->value_name);
    if (shorthand) {
        printf("-%c.\n", opt->shorthand);
    } else {
        printf("--%s.\n", opt->name);
    }
}

static void report_invalid_long_option(size_t len, const char opt[len]) {
    printf("error: Invalid option --%.*s.\n", (int) len, opt);
}

static void report_invalid_short_option(char opt) {
    printf("error: Invalid option -%c.\n", opt);
}

static void report_duplicate_option(const struct cmd_option* opt) {
    printf("error: Duplicate option ");
    if (opt->shorthand && opt->name) {
        printf("-%c/--%s.\n", opt->shorthand, opt->name);
    } else if (opt->shorthand) {
        printf("-%c.\n", opt->shorthand);
    } else {
        printf("--%s.\n", opt->name);
    }
}

static void report_missing_positional(const struct cmd* cmd, size_t i) {
    printf("error: Missing required positional argument <%s>.\n", cmd->leaf.positionals[i].value_name);
}

static void report_superficial_positional(size_t len, const char pos[len]) {
    printf("error: Superficial positional argument '%.*s'.\n", (int) len, pos);
}

static void report_value_parse_error(struct parser* p, enum value_parse_status status) {
    switch (status) {
        case VALUE_PARSE_SUCCESS:
            assert(false);
        case VALUE_PARSE_UNEXPECTED_END:
        case VALUE_PARSE_UNEXPECTED_CHARACTER:
            report_unexpected_character(p);
            break;
        default:
            printf("error: %s.\n", value_parse_status_to_str(status));
    }
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
        report_unexpected_character(p);
        return false;
    }

    for (size_t i = 0; options && cmd_option_is_valid(&options[i]); ++i) {
        const struct cmd_option* opt = &options[i];
        if (strncmp(opt->name, option, option_len) != 0)
            continue;

        if (result->options[i].present) {
            report_duplicate_option(opt);
            return false;
        }

        if (!opt->value_name) {
            result->options[i].value.as_bool = true;
            result->options[i].present = true;
            return true;
        }

        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            report_missing_argument(opt, false);
            return true;
        }

        union value_data value;
        enum value_parse_status status = value_parse(&value, p, opt->value_type);
        if (status != VALUE_PARSE_SUCCESS) {
            report_value_parse_error(p, status);
            return false;
        }
        result->options[i].value = value;
        result->options[i].present = true;
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

            if (result->options[i].present) {
                report_duplicate_option(opt);
                return false;
            }

            ++p->offset;

            if (!opt->value_name) {
                result->options[i].value.as_bool = true;
                result->options[i].present = true;
                goto next_option;
            }

            parser_skip_ws(p);
            if (parser_is_at_end(p)) {
                report_missing_argument(opt, true);
                return false;
            }

            union value_data value;
            enum value_parse_status status = value_parse(&value, p, opt->value_type);
            if (status != VALUE_PARSE_SUCCESS) {
                report_value_parse_error(p, status);
                return false;
            }
            result->options[i].value = value;
            result->options[i].present = true;
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

    // Note, correctly zero-intializes options.
    result->options = calloc(num_optionals, sizeof(struct cmd_parsed_optional));
    result->positionals = calloc(min_positionals, sizeof(union value_data));

    while (true) {
        parser_skip_ws(p);
        if (parser_is_at_end(p)) {
            break;
        } else if (parser_peek(p) != '-') {
            // Parse a positional argument.
            size_t index = result->positionals_len;
            if (index == max_positionals && !variadic) {
                // Attempt to parse something just for error reporting.
                const char* positional = parser_remaining(p);
                size_t positional_len = parser_eat_word(p);
                if (positional_len == 0) {
                    report_unexpected_character(p);
                    return false;
                }

                report_superficial_positional(positional_len, positional);
                return false;
            }

            size_t positional = index >= max_positionals ? max_positionals - 1 : index;
            union value_data value;
            enum value_type type = cmd->leaf.positionals[positional].value_type;
            enum value_parse_status status = value_parse(&value, p, type);
            if (status != VALUE_PARSE_SUCCESS) {
                report_value_parse_error(p, status);
                return false;
            }

            if (++result->positionals_len >= min_positionals) {
                // TODO: More efficient realloc
                result->positionals = realloc(result->positionals, result->positionals_len * sizeof(union value_data));
            }
            result->positionals[index] = value;
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
            report_unexpected_character(p);
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
            if (options[i].value_type == VALUE_TYPE_STR && result->options[i].present) {
                free(result->options[i].value.as_str);
            }
        }

        const struct cmd_positional* pos = result->matched_command->leaf.positionals;
        for (size_t i = 0; i < result->positionals_len; ++i) {
            if (pos->value_type == VALUE_TYPE_STR) {
                free(result->positionals[i].as_str);
            }
            if (cmd_positional_is_valid(pos + 1)) {
                ++pos;
            }
        }
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
