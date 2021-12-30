#include "interpreter.h"
#include "command.h"
#include "commands/commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

void interpreter_init(struct interpreter* interp) {
    interp->quit = false;
    conn_init(&interp->conn);
}

void interpreter_deinit(struct interpreter* interp) {
    conn_close(&interp->conn);
}

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]) {
    struct cmd_parser cmdp;
    cmd_parser_init(&cmdp, commands, len, line);

    if (cmd_parse(&cmdp) && cmdp.matched_command) {
        switch (cmdp.matched_command->type) {
            case CMD_TYPE_LEAF: {
                command_handler_t handler = cmdp.matched_command->leaf.payload;
                handler(interp, cmdp.positionals_len, (const char* const*) cmdp.positionals);
                break;
            }
            case CMD_TYPE_DIRECTORY:
                // TODO: Print help for the directory itself?
                break;
        }
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
