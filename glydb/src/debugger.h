#ifndef _GLYDB_DEBUGGER_H
#define _GLYDB_DEBUGGER_H

#include "connection.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct debugger {
    bool quit;
    struct connection conn;
};

void debugger_init(struct debugger* dbg);

void debugger_deinit(struct debugger* dbg);

void debugger_do_line(struct debugger* dbg, size_t len, const char line[len]);

void debugger_repl(struct debugger* dbg);

bool debugger_require_connection(struct debugger* dbg);

// Invoke a remove command, encoded as a BDBP packet. This function handles both
// sending and receiving: When the function returns success (`false`), `buf` is
// filled with the data returned from the currently connected device. If `true` is
// returned instead, the debugger has already printed a relevant error message, and
// so the handler function should just exit.
bool debugger_exec_cmd(struct debugger* dbg, uint8_t* buf);

#endif
