#include "connection.h"
#include "interpreter.h"

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    struct interpreter interp;
    interpreter_init(&interp);
    interpreter_repl(&interp);
    interpreter_deinit(&interp);
    return EXIT_SUCCESS;
}
