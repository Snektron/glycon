#ifndef _GLYDB_INTERPRETER_H
#define _GLYDB_INTERPRETER_H

#include <stddef.h>
#include <stdbool.h>

struct interpreter {
    bool quit;
};

void interpreter_init(struct interpreter* interp);

void interpreter_do_line(struct interpreter* interp, size_t len, const char line[len]);

void interpreter_repl(struct interpreter* interp);

#endif
