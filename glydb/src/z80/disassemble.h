#ifndef GLYDB_SRC_Z80_DISASSEMBLE_H
#define GLYDB_SRC_Z80_DISASSEMBLE_H

#include "z80/z80.h"

void z80_disassemble(struct z80_inst* inst, size_t len, const uint8_t code[len]);

#endif
