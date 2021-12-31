#include "connection.h"
#include "debugger.h"

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    struct debugger dbg;
    debugger_init(&dbg);
    debugger_repl(&dbg);
    debugger_deinit(&dbg);
    return EXIT_SUCCESS;
}
