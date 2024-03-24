#include "connection.h"
#include "debugger.h"

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    const char* initial_port = NULL;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            puts("usage: glydb [-h|--help] [port]");
            puts("options:");
            puts("-h, --help    Show this message and exit.");
            puts("[port]        Port to connect to, for example /dev/ttyUSB0.");
            return EXIT_SUCCESS;
        } else if (!initial_port) {
            initial_port = arg;
        } else {
            fprintf(stderr, "error: unexpected argument '%s'\n", arg);
        }
    }

    struct debugger dbg;
    debugger_init(&dbg, initial_port);
    debugger_repl(&dbg);
    debugger_deinit(&dbg);
    return EXIT_SUCCESS;
}
