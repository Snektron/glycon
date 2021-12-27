#include "connection.h"
#include "interpreter.h"

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     fprintf(stderr, "usage: %s <device>\n", argv[0]);
    //     return EXIT_FAILURE;
    // }

    // struct connection conn;
    // enum conn_status err = conn_open_serial(&conn, argv[1]);
    // switch (err) {
    //     case CONN_ERR_SERIAL_OPEN:
    //         fprintf(stderr, "error: failed to open serial device '%s'\n", argv[1]);
    //         return EXIT_FAILURE;
    //     case CONN_ERR_SERIAL_ATTRIBS:
    //         fprintf(stderr, "error: failed to configure serial device '%s'\n", argv[1]);
    //         return EXIT_FAILURE;
    //     case CONN_OK:
    //         {}
    // }

    // Wait for the device to reset itself and start up again.
    // sleep(1);

    struct interpreter interp;
    interpreter_init(&interp);

    interpreter_repl(&interp);

    // conn_close(&conn);
    return EXIT_SUCCESS;
}
