#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    if (debugger_require_connection(dbg))
        return;

    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;
    uint8_t packet[] = {
        BDBP_CMD_WRITE,
        address >> 8,
        address & 0xFF,
        data
    };

    int result = conn_write_all(&dbg->conn, 4, packet);
    if (result < 0) {
        printf("write error: %s.\n", strerror(errno));
        return;
    }

    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        printf("read error: %s.\n", strerror(errno));
    } else {
        printf("device returned: %s.\n", bdbp_status_to_string((enum bdbp_status) result));
    }
}

static void memory_read(struct debugger* dbg, const struct cmd_parse_result* args) {
    if (debugger_require_connection(dbg))
        return;

    uint16_t address = args->positionals[0].as_int;
    uint8_t packet[] = {
        BDBP_CMD_READ,
        address >> 8,
        address & 0xFF,
    };

    int result = conn_write_all(&dbg->conn, 3, packet);
    if (result < 0) {
        printf("write error: %s.\n", strerror(errno));
        return;
    }

    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        printf("read error: %s.\n", strerror(errno));
    } else {
        printf("device returned: %s.\n", bdbp_status_to_string((enum bdbp_status) result));
    }

    result = conn_read_byte(&dbg->conn);
    if (result < 0) {
        printf("read error: %s.\n", strerror(errno));
    } else {
        printf("data: %d.\n", result);
    }
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Help for 'memory write'.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "values", "Value to write."},
            {}
        },
        .payload = memory_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Help for 'memory read'.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to read from."},
            {}
        },
        .payload = memory_read
    }}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Help for 'memory'.",
    {.directory = {memory_commands}}
};
