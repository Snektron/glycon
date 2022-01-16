#ifndef GLYDB_SRC_Z80_Z80_H
#define GLYDB_SRC_Z80_Z80_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

enum z80_mnemoric {
    Z80_INVALID,
    Z80_ADC,
    Z80_ADD,
    Z80_AND,
    Z80_BIT,
    Z80_CALL,
    Z80_CCF,
    Z80_CP,
    Z80_CPD,
    Z80_CPDR,
    Z80_CPI,
    Z80_CPIR,
    Z80_CPL,
    Z80_DAA,
    Z80_DEC,
    Z80_DI,
    Z80_DJNZ,
    Z80_EI,
    Z80_EX,
    Z80_EXX,
    Z80_HALT,
    Z80_IM,
    Z80_IN,
    Z80_INC,
    Z80_IND,
    Z80_INDR,
    Z80_INI,
    Z80_INIR,
    Z80_JP,
    Z80_JR,
    Z80_LD,
    Z80_LDD,
    Z80_LDDR,
    Z80_LDI,
    Z80_LDIR,
    Z80_NEG,
    Z80_NOP,
    Z80_OR,
    Z80_OTDR,
    Z80_OTIR,
    Z80_OUT,
    Z80_OUTD,
    Z80_OUTI,
    Z80_POP,
    Z80_PUSH,
    Z80_RES,
    Z80_RET,
    Z80_RETI,
    Z80_RETN,
    Z80_RL,
    Z80_RLA,
    Z80_RLC,
    Z80_RLCA,
    Z80_RLD,
    Z80_RR,
    Z80_RRA,
    Z80_RRC,
    Z80_RRCA,
    Z80_RRD,
    Z80_RST,
    Z80_SBC,
    Z80_SCF,
    Z80_SET,
    Z80_SLA,
    Z80_SLL,
    Z80_SRA,
    Z80_SRL,
    Z80_SUB,
    Z80_XOR
};

enum z80_reg8 {
    Z80_R8_A,
    Z80_R8_B,
    Z80_R8_C,
    Z80_R8_D,
    Z80_R8_E,
    Z80_R8_H,
    Z80_R8_L,

    Z80_R8_F,

    Z80_R8_I,
    Z80_R8_R,

    Z80_R8_IXL,
    Z80_R8_IXH,
    Z80_R8_IYL,
    Z80_R8_IYH
};

enum z80_reg16 {
    Z80_R16_AF,
    Z80_R16_BC,
    Z80_R16_DE,
    Z80_R16_HL,

    Z80_R16_IX,
    Z80_R16_IY,

    Z80_R16_SP
};

enum z80_status {
    Z80_STATUS_NZ,
    Z80_STATUS_Z,
    Z80_STATUS_NC,
    Z80_STATUS_C,
    Z80_STATUS_PO,
    Z80_STATUS_PE,
    Z80_STATUS_P,
    Z80_STATUS_M
};

enum z80_im {
    Z80_IM_0,
    Z80_IM_1,
    Z80_IM_2,
    Z80_IM_01
};

enum z80_op_type {
    Z80_OP_NONE, // Used to mark absence
    Z80_OP_R8,
    Z80_OP_R8_DEREF, // Used for in (c) etc
    Z80_OP_R16,
    Z80_OP_R16_DEREF, // Used for (hl) etc
    Z80_OP_IX_REL, // Used for (ix+x)
    Z80_OP_IY_REL, // Used for (iy+x)
    Z80_OP_STATUS, // Used for condition codes
    Z80_OP_AF_SHADOW, // The AF shadow register, for `ex af, af'`.
    Z80_OP_U8,
    Z80_OP_U8_DEREF, // (n)
    Z80_OP_U16,
    Z80_OP_U16_DEREF, // (nn)
    Z80_OP_JMP_REL, // Relative address for jmp, -126 to 129
    Z80_OP_IM,
};

struct z80_op {
    enum z80_op_type type;
    union {
        enum z80_reg8 reg8;
        enum z80_reg16 reg16;
        enum z80_status status;
        uint8_t imm8;
        uint16_t imm16;
        int16_t jmp_disp; // -126 to +129.
        int8_t displacement;
        enum z80_im im;
    };
};

#define Z80_MAX_OPERANDS (3)

struct z80_inst {
    enum z80_mnemoric mnemoric;
    struct z80_op operands[Z80_MAX_OPERANDS];
    // Opcode size in bytes.
    uint8_t size;
};

const char* z80_mnemoric_to_str(enum z80_mnemoric mnemoric);

const char* z80_reg8_to_str(enum z80_reg8 reg8);

const char* z80_reg16_to_str(enum z80_reg16 reg16);

const char* z80_status_to_str(enum z80_status status);

const char* z80_im_to_str(enum z80_im im);

void z80_print_inst(const struct z80_inst* inst, FILE* stream);

#endif
