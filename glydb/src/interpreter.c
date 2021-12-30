#include "interpreter.h"
#include "command/command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

const struct cmd* memory_commands[] = {
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

const struct cmd* commands[] = {
    &(struct cmd){CMD_TYPE_DIRECTORY, "memory", "help for 'memory'", {.directory = {memory_commands}}},
    NULL
};

void interpreter_init(struct interpreter* interp) {
    interp->quit = false;
}

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]) {
    (void) interp;
    struct cmd_parser cmdp;
    cmd_parser_init(&cmdp, commands, len, line);
    cmd_parse(&cmdp);
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
