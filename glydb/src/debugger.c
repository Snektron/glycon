#include "debugger.h"
#include "command.h"
#include "commands/commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

void debugger_init(struct debugger* dbg) {
    dbg->quit = false;
    conn_init(&dbg->conn);
}

void debugger_deinit(struct debugger* dbg) {
    conn_close(&dbg->conn);
}

void debugger_do_line(struct debugger* dbg, size_t len, const char line[len]) {
    struct cmd_parser cmdp;
    cmd_parser_init(&cmdp, commands, len, line);

    if (cmd_parse(&cmdp) && cmdp.matched_command) {
        switch (cmdp.matched_command->type) {
            case CMD_TYPE_LEAF: {
                command_handler_t handler = cmdp.matched_command->leaf.payload;
                handler(dbg, cmdp.positionals_len, (const char* const*) cmdp.positionals);
                break;
            }
            case CMD_TYPE_DIRECTORY:
                // TODO: Print help for the directory itself?
                break;
        }
    }

    cmd_parser_deinit(&cmdp);
}

void debugger_repl(struct debugger* dbg) {
    dbg->quit = false;

    while (!dbg->quit) {
        char* line = readline(prompt);
        if (!line) {
            return;
        } else {
            size_t len = strlen(line);
            debugger_do_line(dbg, len, line);
            free(line);
        }
    }
}

bool debugger_require_connection(struct debugger* dbg) {
    if (!conn_is_open(&dbg->conn)) {
        printf("error: No active connection. Connect to a device using `connection open`.\n");
        return true;
    }

    return false;
}
