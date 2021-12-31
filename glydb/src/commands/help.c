#include "commands/commands.h"
#include "debugger.h"

#include <string.h>
#include <stdio.h>

struct help {
    const struct cmd* const* spec;
    size_t command_len;
    const char* const* command;
};

static void print_command(const struct help* h) {
    const struct cmd* cmd = NULL;
    const struct cmd* const* spec = h->spec;
    for (size_t i = 0; i < h->command_len; ++i) {
        cmd = cmd_match_command(spec, strlen(h->command[i]), h->command[i]);
        if (!cmd) {
            // No match: just print the word
            printf("%s%s", &" "[i == 0], h->command[i]);
        } else {
            // Print the full name for clarity.
            printf("%s%s", &" "[i == 0], cmd->name);
        }

        if (cmd && cmd->type == CMD_TYPE_DIRECTORY) {
            spec = cmd->directory.subcommands;
        } else {
            spec = NULL;
        }
    }
}

static void report_help_for_directory(const struct help* h, const struct cmd* cmd) {
    puts(cmd->help);
    printf("\nSyntax: ");
    print_command(h);
    puts(" <subcommand> [<subcommand options>]\n\nAvailable subcommands:");

    const struct cmd* const* spec = cmd->directory.subcommands;
    for (size_t i = 0; spec[i]; ++i) {
        printf("    %s - %s\n", spec[i]->name, spec[i]->help);
    }
}

static void print_option(const struct cmd_option* opt) {
    if (opt->name && opt->shorthand) {
        printf("-%c/--%s", opt->shorthand, opt->name);
    } else if (opt->name) {
        printf("--%s", opt->name);
    } else {
        printf("-%c", opt->shorthand);
    }

    if (opt->value_name) {
        printf(" <%s>", opt->value_name);
    }
}

static void print_positional(const struct cmd_positional* pos) {
    if ((pos->flags & CMD_OPTIONAL) && (pos->flags & CMD_VARIADIC)) {
        printf("[%s...]", pos->value_name);
    } else if (pos->flags & CMD_VARIADIC) {
        printf("<%s...>", pos->value_name);
    } else if (pos->flags & CMD_OPTIONAL) {
        printf("[%s]", pos->value_name);
    } else {
        printf("<%s>", pos->value_name);
    }
}

static void report_help_for_leaf(const struct help* h, const struct cmd* cmd) {
    puts(cmd->help);
    printf("\nSyntax: ");
    print_command(h);

    bool any_options = false;
    const struct cmd_option* options = cmd->leaf.options;
    for (size_t i = 0; options && cmd_option_is_valid(&options[i]); ++i) {
        printf(" [");
        print_option(&options[i]);
        printf("]");
        any_options = true;
    }

    const struct cmd_positional* positionals = cmd->leaf.positionals;
    for (size_t i = 0; positionals && cmd_positional_is_valid(&positionals[i]); ++i) {
        printf(" ");
        print_positional(&positionals[i]);
        any_options = true;
    }

    puts("");
    if (!any_options)
        return;

    puts("\nUsage:");

    for (size_t i = 0; options && cmd_option_is_valid(&options[i]); ++i) {
        print_option(&options[i]);
        printf("\n    %s\n", options[i].help);
    }

    for (size_t i = 0; positionals && cmd_positional_is_valid(&positionals[i]); ++i) {
        print_positional(&positionals[i]);
        printf("\n    %s\n", positionals[i].help);
    }
}

static void report_help_for_help(const struct help* h) {
    puts("Available commands:");
    for (size_t i = 0; h->spec[i]; ++i) {
        printf("%s\t- %s\n", h->spec[i]->name, h->spec[i]->help);
    }
}

static void report_help(const struct help* h) {
    const struct cmd* cmd = NULL;
    const struct cmd* const* spec = h->spec;
    for (size_t i = 0; i < h->command_len; ++i) {
        cmd = cmd_match_command(spec, strlen(h->command[i]), h->command[i]);

        if (!cmd)
            goto no_such_command;

        switch (cmd->type) {
            case CMD_TYPE_LEAF:
                if (i != h->command_len - 1)
                    goto no_such_command;
                break; // Will exit loop on next iteration anyway.
            case CMD_TYPE_DIRECTORY:
                spec = cmd->directory.subcommands;
                break;
        }
    }

    if (!cmd) {
        report_help_for_help(h);
        return;
    }

    switch (cmd->type) {
        case CMD_TYPE_LEAF:
            report_help_for_leaf(h, cmd);
            return;
        case CMD_TYPE_DIRECTORY:
            report_help_for_directory(h, cmd);
            return;
    }
no_such_command:
    printf("error: invalid command '");
    print_command(h);
    puts("'.");
}

static void help(struct debugger* dbg, size_t len, const char* const args[len]) {
    (void) dbg;
    struct help h = {commands, len, args};
    report_help(&h);
}

const struct cmd command_help = {
    .type = CMD_TYPE_LEAF,
    .name = "help",
    .help = "List available commands or show information about a specific command.",
    {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {"command", "Command to get help for.", CMD_OPTIONAL | CMD_VARIADIC},
            {}
        },
        .payload = help
    }
}};
