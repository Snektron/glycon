#include "connection.h"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <stdbool.h>

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

    // Read blocks until one byte is received, no timeout between characters
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return false;
    }

    return true;
}

enum conn_status conn_open_serial(struct connection* conn, const char* path) {
    conn->fd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
    if (conn->fd == -1) {
        return CONN_ERR_SERIAL_OPEN;
    }

    // TODO: Don't hardcode serial attributes
    if (!set_serial_attribs(conn->fd, B115200, 0)) {
        close(conn->fd);
        return CONN_ERR_SERIAL_ATTRIBS;
    }

    return CONN_OK;
}

void conn_close(struct connection* conn) {
    close(conn->fd);
}
