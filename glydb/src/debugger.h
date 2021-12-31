#ifndef _GLYDB_DEBUGGER_H
#define _GLYDB_DEBUGGER_H

#include "connection.h"

#include <stddef.h>
#include <stdbool.h>

struct debugger {
    bool quit;
    struct connection conn;
};

void debugger_init(struct debugger* dbg);

void debugger_deinit(struct debugger* dbg);

void debugger_do_line(struct debugger* dbg, size_t len, const char line[len]);

void debugger_repl(struct debugger* dbg);

bool debugger_require_connection(struct debugger* dbg);

#endif
