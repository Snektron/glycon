#include "commands/commands.h"
#include "interpreter.h"
#include "connection.h"

#include <stdio.h>

static void connection_open(struct interpreter* interp, size_t len, const char* const args[len]) {
    switch (conn_open_serial(&interp->conn, args[0])) {
        case CONN_OK:
            break;
        case CONN_ERR_SERIAL_OPEN:
            printf("error: Failed to open serial port '%s'.\n", args[0]);
            break;
        case CONN_ERR_SERIAL_ATTRIBS:
            printf("error: Failed to configure serial port '%s'.\n", args[0]);
            break;
        case CONN_ERR_ALREADY_OPEN:
            puts("error: A connection is already open. Close it first with `connection close`.");
            break;
    }
}

static void connection_close(struct interpreter* interp, size_t len, const char* const args[len]) {
    conn_close(&interp->conn);
}

static void connection_status(struct interpreter* interp, size_t len, const char* const args[len]) {
    if (conn_is_open(&interp->conn)) {
        printf("Currently connected to serial device on port '%s'.\n", interp->conn.port);
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
