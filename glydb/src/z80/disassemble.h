#ifndef GLYDB_SRC_Z80_DISASSEMBLE_H
#define GLYDB_SRC_Z80_DISASSEMBLE_H

#include "z80/z80.h"

// Disassemble a single instruction from `code`.
// Sets `inst->mnemoric` to `Z80_INVALID` if the instruction could not be decoded.
// `inst->size` is always valid, even if the instruction was not.
void z80_disassemble(struct z80_inst* inst, size_t len, const uint8_t code[len]);

#endif
