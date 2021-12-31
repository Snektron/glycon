#include "commands/commands.h"
#include "debugger.h"

static void quit(struct debugger* dbg, size_t len, const char* const args[len]) {
    (void) len;
    (void) args;
    dbg->quit = true;
}

const struct cmd command_quit = {
    .type = CMD_TYPE_LEAF,
    .name = "quit",
    .help = "Quit the debugger.",
    {.leaf = {
        .options = NULL,
        .positionals = NULL,
        .payload = quit
    }
}};
