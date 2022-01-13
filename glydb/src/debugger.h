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

// Invoke a remove command, encoded as a BDBP packet. This function handles both
// sending and receiving: When the function returns success (`false`), `buf` is
// filled with the data returned from the currently connected device. If `true` is
// returned instead, the debugger has already printed a relevant error message, and
// so the handler function should just exit.
bool debugger_exec_cmd(struct debugger* dbg, uint8_t* buf);

void debugger_print_error(struct debugger* dbg, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

// Write a buffer of arbitrary length to the target memory. This will split up
// the write into multiple packets as needed.
bool debugger_write_memory(struct debugger* dbg, uint16_t address, size_t len, const uint8_t buffer[len]);

// Read a buffer of arbitrary length from the target memory. This will split up
// the read into multiple packets as needed.
bool debugger_read_memory(struct debugger* dbg, uint16_t address, size_t len, uint8_t buffer[len]);

#endif
