#ifndef _GLYDB_DEBUGGER_H
#define _GLYDB_DEBUGGER_H

#include "connection.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct debugger {
    bool quit;
    struct connection conn;
    // Address-space sized buffer that can be used to store data for reading/writing.
    uint8_t* scratch;
};

void debugger_init(struct debugger* dbg);

void debugger_deinit(struct debugger* dbg);

void debugger_do_line(struct debugger* dbg, size_t len, const char line[len]);

void debugger_repl(struct debugger* dbg);

bool debugger_require_connection(struct debugger* dbg);

void debugger_print_error(struct debugger* dbg, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

#endif
