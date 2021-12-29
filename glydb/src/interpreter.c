#include "interpreter.h"
#include "command/command.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

struct cmd memory_commands[] = {
    {CMD_TYPE_LEAF, "write", "help for 'memory write'"},
    {CMD_TYPE_LEAF, "read", "help for 'memory read'"},
    {}
};

struct cmd commands[] = {
    {CMD_TYPE_DIRECTORY, "memory", "help for 'memory'", {{memory_commands}}},
    {}
};

void interpreter_init(struct interpreter* interp) {
    interp->quit = false;
}

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]) {
    (void) interp;
    cmd_parse(commands, len, line);
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
