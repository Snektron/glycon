#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstdio>

// https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
bool set_attribs(int fd, speed_t speed, int parity) {
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

    tty.c_oflag &= ~OPOST;

    // Read blocks until one byte is received, half-second timeout between characters
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <device>\n";
        return EXIT_FAILURE;
    }

    int device_fd = open(argv[1], O_RDWR | O_NOCTTY | O_SYNC);
    if (!device_fd) {
        std::cerr << "error: failed to open '" << argv[1] << "'\n";
        return EXIT_FAILURE;
    }

    if (!set_attribs(device_fd, B115200, 0)) {
        puts("Error: failed to set device attributes");
        close(device_fd);
        return EXIT_FAILURE;
    }

    sleep(1);

    while (true) {
        const char msg[] = "1";
        ssize_t w = write(device_fd, msg, sizeof msg - 1);
        if (w >= 0) {
            std::cout << "wrote " << w << " bytes" << std::endl;
        } else {
            std::cerr << "error: write returned " << w << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }

    close(device_fd);
    return EXIT_SUCCESS;
}
