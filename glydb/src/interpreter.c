#include "interpreter.h"
#include "command/command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

static void handle_help(size_t len, const char* const* args);

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "help for 'memory write'", {.leaf = {
        .options = (struct cmd_option[]){
            {"count", 'c', "the number of elements to write", "amount"},
            {"size", 's', "the size of an invididual element", "amount"},
            {"dry", 'd', "do not actually perform the write"},
            {}
        },
        .positionals = (struct cmd_positional[]){
            {"address", "the address to start writing at"},
            {"values", "values to write", CMD_OPTIONAL | CMD_VARIADIC},
            {}
        },
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "help for 'memory read'", {.leaf = {}}},
    NULL
};

static const struct cmd* commands[] = {
    &(struct cmd){CMD_TYPE_DIRECTORY, "memory", "help for 'memory'", {.directory = {memory_commands}}},
    &(struct cmd){CMD_TYPE_LEAF, "help", "help for 'help'", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {"command", "command to get help for", CMD_OPTIONAL | CMD_VARIADIC},
            {}
        },
        .payload = handle_help
    }}},
    NULL
};

static void handle_help(size_t len, const char* const* args) {
    cmd_report_help(commands, len, args);
}

void interpreter_init(struct interpreter* interp) {
    interp->quit = false;
}

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]) {
    (void) interp;
    struct cmd_parser cmdp;
    cmd_parser_init(&cmdp, commands, len, line);

    if (
        cmd_parse(&cmdp)
        && cmdp.matched_command
        && cmdp.matched_command->type == CMD_TYPE_LEAF
        && cmdp.matched_command->leaf.payload
    ) {
        ((void (*)(size_t, const char* const*)) cmdp.matched_command->leaf.payload)(cmdp.positionals_len, (const char* const*) cmdp.positionals);
    }

    cmd_parser_deinit(&cmdp);
}

void interpreter_repl(struct interpreter* interp) {
    interp->quit = false;

    while (!interp->quit) {
        char* line = readline(prompt);
        if (!line) {
            return;
        } else {
            size_t len = strlen(line);
            interpreter_do_line(interp, len, line);
            free(line);
        }
    }
}
