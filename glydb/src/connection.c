#include "connection.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

// https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
static bool set_serial_attribs(int fd, speed_t speed, int parity) {
    struct termios tty = {};

    if (tcgetattr(fd, &tty) != 0) {
        return false;
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // Turn of input processing
    tty.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IXOFF | IXANY);
    // Turn of output processing
    tty.c_oflag = 0;
    // Turn of line processing
    tty.c_lflag = ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    // Disable pararity checking, clear char size mask, force 8 bit char size mask
    tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
    tty.c_cflag |= CS8 | CLOCAL | CREAD | parity;

    // Reads block until a 2-second timeout has been elapsed.
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 20;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return false;
    }

    return true;
}

void conn_init(struct connection* conn) {
    conn->port = NULL;
    conn->fd = -1;
}

bool conn_open_serial(struct connection* conn, const char* path) {
    assert(!conn_is_open(conn));

    int fd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        return false;
    }

    // TODO: Don't hardcode serial attributes
    if (!set_serial_attribs(fd, B115200, 0)) {
        close(fd);
        return false;
    }

    // Wait until the arduino has started
    sleep(1);

    conn->fd = fd;
    conn->port = strdup(path);
    return true;
}

void conn_close(struct connection* conn) {
    if (conn->fd != -1) {
        free(conn->port);
        close(conn->fd);
    }
    conn->fd = -1;
    conn->port = NULL;
}

bool conn_is_open(struct connection* conn) {
    return conn->fd != -1;
}

int conn_write_byte(struct connection* conn, uint8_t byte) {
    ssize_t w = write(conn->fd, &byte, 1);
    if (w < 0) {
        return -1;
    } else if (w == 0) {
        errno = ETIMEDOUT;
        return -1;
    }

    return 0;
}

int conn_read_byte(struct connection* conn) {
    uint8_t byte;
    ssize_t r = read(conn->fd, &byte, 1);
    if (r < 0) {
        return -1;
    } else if (r == 0) {
        errno = ETIMEDOUT;
        return -1;
    } else {
        return byte;
    }
}

int conn_write_all(struct connection* conn, size_t len, const uint8_t data[len]) {
    for (size_t i = 0; i < len; ++i) {
        if (conn_write_byte(conn, data[i]) < 0) {
            return -1;
        }
    }

    return 0;
}

// int conn_write_all(struct connection* conn, size_t len, const uint8_t data[len]) {
//     size_t offset = 0;
//     while (offset < len) {
//         ssize_t written = write(conn->fd, &data[offset], len - offset);
//         if (written < 0) {
//             return -1;
//         } else if (written == 0) {
//             errno = ETIMEDOUT;
//             return -1;
//         }
//     }

//     return 0;
// }
