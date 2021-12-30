#ifndef _GLYDB_CONNECTION_H
#define _GLYDB_CONNECTION_H

#include <stdbool.h>

struct connection {
    char* port;
    int fd;
};

enum conn_status {
    CONN_OK,
    CONN_ERR_SERIAL_OPEN,
    CONN_ERR_SERIAL_ATTRIBS,
    CONN_ERR_ALREADY_OPEN
};

// Initialize a closed connection.
void conn_init(struct connection* conn);

// Open a connection to a serial device `path`, which for example could look like `/dev/ttyUSB0`.
// The device is communicated with using 115200 baud, 8 bits, 1 stop bit and no parity.
// Returns:
// - CONN_OK when the device was opened and configured successfully.
// - CONN_ERR_SERIAL_OPEN when `path` could not be opened.
// - CONN_ERR_SERIAL_ATTRIBS when the device could not be configured properly.
// - CONN_ERR_ALREADY_OPEN when the connection is already connected somewhere.
enum conn_status conn_open_serial(struct connection* conn, const char* path);

// Close a connection.
void conn_close(struct connection* conn);

// Check if this connection is currently connected somewhere
bool conn_is_open(struct connection* conn);

#endif
