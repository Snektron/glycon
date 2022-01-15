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

// A structure describing the target location of some amount of bytes that needs
// to be written. Bytes themselves are stored externally.
struct debugger_write_op {
    uint16_t address;
    size_t len;
};

struct debugger_load_file_options {
    const char* path;
    const char* ext_override;
    uint16_t relocation;
};

bool debugger_load_file(struct debugger* dbg, const struct debugger_load_file_options* opts, struct debugger_write_op** ops, uint8_t* buffer);

#endif
