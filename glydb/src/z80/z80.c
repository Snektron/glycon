#include "z80/z80.h"

#include <assert.h>
#include <stdbool.h>

static const char* z80_mnemoric_str_table[] = {
    [Z80_INVALID] = "(invalid)",
    [Z80_ADC] = "adc",
    [Z80_ADD] = "add",
    [Z80_AND] = "and",
    [Z80_BIT] = "bit",
    [Z80_CALL] = "call",
    [Z80_CCF] = "ccf",
    [Z80_CP] = "cp",
    [Z80_CPD] = "cpd",
    [Z80_CPDR] = "cpdr",
    [Z80_CPI] = "cpi",
    [Z80_CPIR] = "cpir",
    [Z80_CPL] = "cpl",
    [Z80_DAA] = "daa",
    [Z80_DEC] = "dec",
    [Z80_DI] = "di",
    [Z80_DJNZ] = "djnz",
    [Z80_EI] = "ei",
    [Z80_EX] = "ex",
    [Z80_EXX] = "exx",
    [Z80_HALT] = "halt",
    [Z80_IM] = "im",
    [Z80_IN] = "in",
    [Z80_INC] = "inc",
    [Z80_IND] = "ind",
    [Z80_INDR] = "indr",
    [Z80_INI] = "ini",
    [Z80_INIR] = "inir",
    [Z80_JP] = "jp",
    [Z80_JR] = "jr",
    [Z80_LD] = "ld",
    [Z80_LDD] = "ldd",
    [Z80_LDDR] = "lddr",
    [Z80_LDI] = "ldi",
    [Z80_LDIR] = "ldir",
    [Z80_NEG] = "neg",
    [Z80_NOP] = "nop",
    [Z80_OR] = "or",
    [Z80_OTDR] = "otdr",
    [Z80_OTIR] = "otir",
    [Z80_OUT] = "out",
    [Z80_OUTD] = "outd",
    [Z80_OUTI] = "outi",
    [Z80_POP] = "pop",
    [Z80_PUSH] = "push",
    [Z80_RES] = "res",
    [Z80_RET] = "ret",
    [Z80_RETI] = "reti",
    [Z80_RETN] = "retn",
    [Z80_RL] = "rl",
    [Z80_RLA] = "rla",
    [Z80_RLC] = "rlc",
    [Z80_RLCA] = "rlca",
    [Z80_RLD] = "rld",
    [Z80_RR] = "rr",
    [Z80_RRA] = "rra",
    [Z80_RRC] = "rrc",
    [Z80_RRCA] = "rrca",
    [Z80_RRD] = "rrd",
    [Z80_RST] = "rst",
    [Z80_SBC] = "sbc",
    [Z80_SCF] = "scf",
    [Z80_SET] = "set",
    [Z80_SLA] = "sla",
    [Z80_SLL] = "sll",
    [Z80_SRA] = "sra",
    [Z80_SRL] = "srl",
    [Z80_SUB] = "sub",
    [Z80_XOR] = "xor",
};

static const char* z80_reg8_str_table[] = {
    [Z80_R8_A] = "a",
    [Z80_R8_B] = "b",
    [Z80_R8_C] = "c",
    [Z80_R8_D] = "d",
    [Z80_R8_E] = "e",
    [Z80_R8_H] = "h",
    [Z80_R8_L] = "l",
    [Z80_R8_F] = "f",
    [Z80_R8_I] = "i",
    [Z80_R8_R] = "r",
    [Z80_R8_IXL] = "ixl",
    [Z80_R8_IXH] = "ixh",
    [Z80_R8_IYL] = "iyl",
    [Z80_R8_IYH] = "ihl"
};

static const char* z80_reg16_str_table[] = {
    [Z80_R16_AF] = "af",
    [Z80_R16_BC] = "bc",
    [Z80_R16_DE] = "de",
    [Z80_R16_HL] = "hl",
    [Z80_R16_IX] = "ix",
    [Z80_R16_IY] = "iy",
    [Z80_R16_SP] = "sp"
};

static const char* z80_status_str_table[] = {
    [Z80_STATUS_NZ] = "nz",
    [Z80_STATUS_Z] = "z",
    [Z80_STATUS_NC] = "nc",
    [Z80_STATUS_C] = "c",
    [Z80_STATUS_PO] = "po",
    [Z80_STATUS_PE] = "pe",
    [Z80_STATUS_P] = "p",
    [Z80_STATUS_M] = "m"
};

static const char* z80_im_str_table[] = {
    [Z80_IM_0] = "0",
    [Z80_IM_1] = "1",
    [Z80_IM_2] = "2",
    [Z80_IM_01] = "0/1",
};

const char* z80_mnemoric_to_str(enum z80_mnemoric mnemoric) {
    return z80_mnemoric_str_table[mnemoric];
}

const char* z80_reg8_to_str(enum z80_reg8 reg8) {
    return z80_reg8_str_table[reg8];
}

const char* z80_reg16_to_str(enum z80_reg16 reg16) {
    return z80_reg16_str_table[reg16];
}

const char* z80_status_to_str(enum z80_status status) {
    return z80_status_str_table[status];
}

const char* z80_im_to_str(enum z80_im im) {
    return z80_im_str_table[im];
}

// Print a single operand in human-readable form to a file stream.
static void print_operand(const struct z80_op* op, FILE* stream) {
    switch (op->type) {
        case Z80_OP_NONE:
            assert(false);
        case Z80_OP_R8:
            fprintf(stream, "%s", z80_reg8_to_str(op->reg8));
            break;
        case Z80_OP_R8_DEREF:
            fprintf(stream, "(%s)", z80_reg8_to_str(op->reg8));
            break;
        case Z80_OP_R16:
            fprintf(stream, "%s", z80_reg16_to_str(op->reg16));
            break;
        case Z80_OP_R16_DEREF:
            fprintf(stream, "(%s)", z80_reg16_to_str(op->reg16));
            break;
        case Z80_OP_IX_REL:
            if (op->displacement < 0) {
                fprintf(stream, "(ix - %d)", -op->displacement);
            } else {
                fprintf(stream, "(ix + %d)", op->displacement);
            }
            break;
        case Z80_OP_IY_REL:
            if (op->displacement < 0) {
                fprintf(stream, "(iy - %d)", -op->displacement);
            } else {
                fprintf(stream, "(iy + %d)", op->displacement);
            }
            break;
        case Z80_OP_STATUS:
            fprintf(stream, "%s", z80_status_to_str(op->status));
            break;
        case Z80_OP_AF_SHADOW:
            fprintf(stream, "af'");
            break;
        case Z80_OP_U8:
            fprintf(stream, "0x%X", op->imm8);
            break;
        case Z80_OP_U8_DEREF:
            fprintf(stream, "(0x%X)", op->imm8);
            break;
        case Z80_OP_U16:
            fprintf(stream, "0x%X", op->imm16);
            break;
        case Z80_OP_U16_DEREF:
            fprintf(stream, "(0x%X)", op->imm16);
            break;
        case Z80_OP_JMP_REL:
            fprintf(stream, "%d", op->jmp_disp);
            break;
        case Z80_OP_IM:
            fprintf(stream, "%s", z80_im_to_str(op->im));
            break;
    }
}

void z80_print_inst(const struct z80_inst* inst, FILE* stream) {
    fprintf(stream, "%s", z80_mnemoric_to_str(inst->mnemoric));
    for (size_t i = 0; i < Z80_MAX_OPERANDS; ++i) {
        if (inst->operands[i].type == Z80_OP_NONE)
            break;
        if (i == 0)
            fprintf(stream, " ");
        else
            fprintf(stream, ", ");
        print_operand(&inst->operands[i], stream);
    }
}
