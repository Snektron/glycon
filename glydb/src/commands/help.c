#include "commands/commands.h"
#include "interpreter.h"

#include <string.h>
#include <stdio.h>

static void report_help(const struct cmd* const* spec, size_t len, const char* const* command) {
    const struct cmd* cmd = NULL;
    size_t i = 0;
    for (; i < len; ++i) {
        size_t j = 0;
        for (; spec[j]; ++j) {
            if (strncmp(command[i], spec[j]->name, strlen(command[i])) == 0)
                break;
        }

        if (!spec[j])
            goto no_such_command;

        cmd = spec[j];
        switch (spec[j]->type) {
            case CMD_TYPE_LEAF:
                if (i != len - 1)
                    goto no_such_command;
                break; // Will exit loop on next iteration anyway.
            case CMD_TYPE_DIRECTORY:
                spec = cmd->directory.subcommands;
                break;
        }
    }

    if (!cmd) {
        printf("TODO: Help for help?\n");
        return;
    }

    printf("TODO: Help for %s\n", cmd->name);
    return;
no_such_command:
    printf("error: invalid command '");
    for (size_t i = 0; i < len; ++i) {
        printf("%s%s", &" "[i == 0], command[i]);
    }
    puts("'");
}

static void help(struct interpreter* interp, size_t positionals_len, const char* const* positionals) {
    (void) interp;
    report_help(commands, positionals_len, positionals);
}

const struct cmd command_help = {
    .type = CMD_TYPE_LEAF,
    .name = "help",
    .help = "help for 'help'",
    {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {"command", "command to get help for", CMD_OPTIONAL | CMD_VARIADIC},
            {}
        },
        .payload = help
    }
}};
