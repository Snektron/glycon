#ifndef GLYDB_SRC_TARGET_H
#define GLYDB_SRC_TARGET_H

#include "common/glycon.h"
#include "debugger.h"

#include <stdint.h>

// The functions in this header are used for medium-level target functionality that is useful
// for implementing different commands.

// Invoke a remove command, encoded as a BDBP packet. This function handles both
// sending and receiving: When the function returns success (`false`), `buf` is
// filled with the data returned from the currently connected device. If `true` is
// returned instead, the debugger has already printed a relevant error message, and
// so the handler function should just exit.
bool target_exec_cmd(struct debugger* dbg, uint8_t* buf);

// Write a buffer of arbitrary length to the target memory. This will split up
// the write into multiple packets as needed.
bool target_write_memory(struct debugger* dbg, gly_addr_t address, size_t len, const uint8_t buffer[]);

// Write a buffer of arbitrary length to the target flash. This will split up
// the write into multiple packets as needed.
// Note that this function does not handle erasing flash.
bool target_write_flash(struct debugger* dbg, gly_addr_t address, size_t len, const uint8_t buffer[]);

// Read a buffer of arbitrary length from the target memory. This will split up
// the read into multiple packets as needed.
// This function can also be used to read out flash memory areas.
bool target_read_memory(struct debugger* dbg, gly_addr_t address, size_t len, uint8_t buffer[]);

#endif
