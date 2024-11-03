#ifndef _GLYDB_CONNECTION_H
#define _GLYDB_CONNECTION_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// This structure represents a connection to a coprocessor.
struct connection {
    // Path of the port of this connection.
    // `NULL` if no current connection.
    char* port;
    // File descriptor of the connection.
    // -1 if no current connection.
    int fd;
};

// Initialize a closed connection.
void conn_init(struct connection* conn);

// Attempt to open a connection to a serial device `path`, which for example could
// look like `/dev/ttyUSB0`. The device is communicated with using 115200 baud,
// 8 bits, 1 stop bit and no parity.
// If successfull, returns `true`, otherwise returns `false` and sets `errno` to indicate
// the error.
bool conn_open_serial(struct connection* conn, const char* path);

// Close a connection.
void conn_close(struct connection* conn);

// Check if this connection is currently initialized as a connected to somewhere.
// Note that this doesn't check whether the other end of the connection is actually
// online.
bool conn_is_open(struct connection* conn);

// Write a single byte to the connection.
// Returns -1 on error, in which case `errno` holds a describing error.
int conn_write_byte(struct connection* conn, uint8_t byte);

// Read a single byte from the connection.
// Returns -1 on error, in which case `errno` holds a describing error.
int conn_read_byte(struct connection* conn);

// Write all bytes in `data` to the connection.
// Returns -1 on error, in which case `errno` holds a describing error.
int conn_write_all(struct connection* conn, size_t len, const uint8_t data[]);

#endif
