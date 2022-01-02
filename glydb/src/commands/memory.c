#include "commands/commands.h"
#include "debugger.h"

#include <stdio.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    printf("address: %li\n", args->positionals[0].as_int);
    printf("size: %li\n", args->positionals[1].as_int);
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Help for 'memory write'.", {.leaf = {
        .options = (struct cmd_option[]){
            {"count", 'c', VALUE_TYPE_INT, "amount", "Number of repetitions to write."},
            {"size", 's', VALUE_TYPE_INT, "amount", "The size of an invididual element."},
            {}
        },
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to start writing at."},
            {VALUE_TYPE_INT, "values", "Value(s) to write.", CMD_VARIADIC},
            {}
        },
        .payload = memory_write
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
