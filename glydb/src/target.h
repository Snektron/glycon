#ifndef GLYDB_SRC_TARGET_H
#define GLYDB_SRC_TARGET_H

#include "debugger.h"

#include <stdint.h>

// Invoke a remove command, encoded as a BDBP packet. This function handles both
// sending and receiving: When the function returns success (`false`), `buf` is
// filled with the data returned from the currently connected device. If `true` is
// returned instead, the debugger has already printed a relevant error message, and
// so the handler function should just exit.
bool target_exec_cmd(struct debugger* dbg, uint8_t* buf);

// Write a buffer of arbitrary length to the target memory. This will split up
// the write into multiple packets as needed.
bool target_write_memory(struct debugger* dbg, uint16_t address, size_t len, const uint8_t buffer[len]);

// Read a buffer of arbitrary length from the target memory. This will split up
// the read into multiple packets as needed.
bool target_read_memory(struct debugger* dbg, uint16_t address, size_t len, uint8_t buffer[len]);

#endif
