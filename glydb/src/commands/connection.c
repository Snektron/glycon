#include "commands/commands.h"
#include "debugger.h"
#include "connection.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

static void connection_open(struct debugger* dbg, size_t len, const char* const args[len]) {
    if (conn_is_open(&dbg->conn)) {
        puts("error: A connection is already open. Close it first with `connection close`.");
    } else if (!conn_open_serial(&dbg->conn, args[0])) {
        printf("error: Failed to open serial port '%s': %s.\n", args[0], strerror(errno));
    }
}

static void connection_close(struct debugger* dbg, size_t len, const char* const args[len]) {
    (void) len;
    (void) args;
    conn_close(&dbg->conn);
}

static void connection_status(struct debugger* dbg, size_t len, const char* const args[len]) {
    (void) len;
    (void) args;
    if (conn_is_open(&dbg->conn)) {
        printf("Currently connected to serial device on port '%s'.\n", dbg->conn.port);
    } else {
        puts("No active connection.");
    }
}

static const struct cmd* connection_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "open", "Open a new connection.", {.leaf = {
        .options = NULL, // TODO: Serial port options?
        .positionals = (struct cmd_positional[]){
            {"port", "The serial port to connect to."},
            {}
        },
        .payload = connection_open
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "close", "Close the currently active connection.", {.leaf = {
        .payload = connection_close
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "status", "Show information about the currently active connection.", {.leaf = {
        .payload = connection_status
    }}},
    NULL
};

const struct cmd command_connection = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "connection",
    .help = "Manage the currently active connection.",
    {.directory = {connection_commands}}
};
