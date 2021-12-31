#include "commands/commands.h"

const struct cmd* commands[] = {
    &command_help,
    &command_quit,
    &command_memory,
    &command_connection,
    &command_ping,
    NULL
};
