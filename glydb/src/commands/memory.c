#include "commands/commands.h"
#include "debugger.h"

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Help for 'memory write'.", {.leaf = {
        .options = (struct cmd_option[]){
            {"count", 'c', "Number of repetitions to write.", "amount"},
            {"size", 's', "The size of an invididual element.", "amount"},
            {}
        },
        .positionals = (struct cmd_positional[]){
            {"address", "The address to start writing at."},
            {"values", "Value(s) to write.", CMD_VARIADIC},
            {}
        },
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Help for 'memory read'.", {.leaf = {}}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Help for 'memory'.",
    {.directory = {memory_commands}}
};
