#include "commands/commands.h"

const struct cmd* commands[] = {
    &command_help,
    &command_quit,
    &command_memory,
    NULL
};
