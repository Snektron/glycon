#include "interpreter.h"
#include "command/command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

struct cmd_option memory_write_opts[] = {
    {"count", 'c', "the number of elements to write", "amount"},
    {"size", 's', "the size of an invididual element", "amount"},
    {"dry", 'd', "do not actually perform the write"},
    {}
};

struct cmd memory_commands[] = {
    {CMD_TYPE_LEAF, "write", "help for 'memory write'", {.leaf = {memory_write_opts, 0, NULL}}},
    {CMD_TYPE_LEAF, "read", "help for 'memory read'", {.leaf = {NULL, 0, NULL}}},
    {}
};

struct cmd commands[] = {
    {CMD_TYPE_DIRECTORY, "memory", "help for 'memory'", {.directory = {memory_commands}}},
    {}
};

void interpreter_init(struct interpreter* interp) {
    interp->quit = false;
}

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]) {
    (void) interp;
    struct cmd_parser cmdp;
    cmd_parser_init(&cmdp, commands, len, line);
    cmd_parse(&cmdp);
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
