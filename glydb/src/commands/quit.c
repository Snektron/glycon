#include "commands/commands.h"
#include "interpreter.h"

void quit(struct interpreter* interp, size_t positionals_len, const char* const* positionals) {
    (void) positionals_len;
    (void) positionals;
    interp->quit = true;
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
