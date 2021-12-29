#include "interpreter.h"
#include "command/command.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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

    char* line = NULL;
    size_t buflen = 0;

    while (!interp->quit) {
        printf("%s", prompt);
        ssize_t len = getline(&line, &buflen, stdin);
        if (len >= 0) {
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = 0;
                interpreter_do_line(interp, (size_t) len - 1, line);
            } else {
                // User didn't end prompt with a newline - add one for them
                puts("");
                interpreter_do_line(interp, (size_t) len, line);
            }
        } else if (errno == 0) {
            // End of input, just shut down.
            // Still print an end-of-line though
            puts("");
            goto exit;
        } else {
            perror("error");
            goto exit;
        }
    }

exit:
    free(line);
}
