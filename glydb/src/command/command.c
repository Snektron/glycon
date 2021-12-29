#include "command/command.h"
#include "command/parser.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

void parse_leaf(const struct cmd* leaf, struct parser* p) {
    printf("Handling leaf %s\n", leaf->name);
}

void parse_cmd(const struct cmd* spec, struct parser* p) {
    parser_skip_ws(p);
    const char* command = parser_remaining(p);
    size_t command_len = parser_eat_verb(p);

    while (command_len > 0 && spec->type != CMD_TYPE_END) {
        const char* name = spec->name;
        if (strncmp(command, name, command_len) != 0) {
            ++spec;
            continue;
        }

        // Matched a (sub)command.
        switch (spec->type) {
            case CMD_TYPE_END:
                assert(false);
            case CMD_TYPE_DIRECTORY:
                parse_cmd(spec->directory.subcommands, p);
                return;
            case CMD_TYPE_LEAF:
                parse_leaf(spec, p);
                return;
        }
    }

    // No commands matched.
    // TODO: Proper handling.
    printf("Invalid (sub)command '%.*s'\n", (int) command_len, command);
}

void cmd_parse(const struct cmd* spec, size_t len, const char line[len]) {
    struct parser p;
    parser_init(&p, len, line);
    parse_cmd(spec, &p);
}

