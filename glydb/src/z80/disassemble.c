#include "z80/disassemble.h"

#include <stdbool.h>
#include <assert.h>

// Disassembler implemented according to http://www.z80.info/decoding.htm

struct disas_ctx {
    struct z80_inst* inst;
    size_t code_len;
    const uint8_t* code;
    union {
        uint8_t full;
        struct __attribute__((packed)) {
            uint8_t z : 3;
            uint8_t y : 3;
            uint8_t x : 2;
        };
        struct __attribute__((packed)) {
            uint8_t : 3;
            uint8_t q : 1;
            uint8_t p : 2;
        };
    } opcode;

    bool out_of_bounds;

    bool is_ix;
    bool is_iy;
    bool is_extended;
    bool is_bit; // can be set simultaneously with is_ix or is_iy

    int8_t displacement;
};

static const enum z80_status cc_tab[] = {
    [0] = Z80_STATUS_NZ,
    [1] = Z80_STATUS_Z,
    [2] = Z80_STATUS_NC,
    [3] = Z80_STATUS_C,
    [4] = Z80_STATUS_PO,
    [5] = Z80_STATUS_PE,
    [6] = Z80_STATUS_P,
    [7] = Z80_STATUS_M
};

static const enum z80_reg16 rp_tab[] = {
    [0] = Z80_R16_BC,
    [1] = Z80_R16_DE,
    [2] = Z80_R16_HL,
    [3] = Z80_R16_SP
};

static const enum z80_reg16 rp2_tab[] = {
    [0] = Z80_R16_BC,
    [1] = Z80_R16_DE,
    [2] = Z80_R16_HL,
    [3] = Z80_R16_AF
};

static const enum z80_im im_tab[] = {
    [0] = Z80_IM_0,
    [1] = Z80_IM_01,
    [2] = Z80_IM_1,
    [3] = Z80_IM_2
};

static const enum z80_mnemoric bli_tab[4][4] = {
    {Z80_LDI, Z80_CPI, Z80_INI, Z80_OUTI},
    {Z80_LDD, Z80_CPD, Z80_IND, Z80_OUTD},
    {Z80_LDIR, Z80_CPIR, Z80_INIR, Z80_OTIR},
    {Z80_LDDR, Z80_CPDR, Z80_INDR, Z80_OTDR}
};

static const enum z80_mnemoric rot_tab[] = {
    Z80_RLC,
    Z80_RRC,
    Z80_RL,
    Z80_RR,
    Z80_SLA,
    Z80_SRA,
    Z80_SLL,
    Z80_SRL
};

static struct z80_op* next_operand(struct disas_ctx* ctx) {
    if (ctx->inst->operands[0].type == Z80_OP_NONE)
        return &ctx->inst->operands[0];
    else if (ctx->inst->operands[1].type == Z80_OP_NONE)
        return &ctx->inst->operands[1];
    else if (ctx->inst->operands[2].type == Z80_OP_NONE)
        return &ctx->inst->operands[2];
    assert(false);
}

static uint8_t read_u8(struct disas_ctx* ctx) {
    if (ctx->out_of_bounds || ctx->inst->size == ctx->code_len) {
        ctx->out_of_bounds = true;
        return 0;
    }

    return ctx->code[ctx->inst->size++];
}

static void op_n(struct disas_ctx* ctx) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_U16;
    op->imm8 = read_u8(ctx);
}

static void op_nn(struct disas_ctx* ctx) {
    struct z80_op* op = next_operand(ctx);
    uint8_t lo = read_u8(ctx);
    uint8_t hi = read_u8(ctx);
    op->type = Z80_OP_U16;
    op->imm16 = (hi << 8) | lo;
}

static void op_n_deref(struct disas_ctx* ctx) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_U8_DEREF;
    op->imm8 = read_u8(ctx);
}

static void op_nn_deref(struct disas_ctx* ctx) {
    struct z80_op* op = next_operand(ctx);
    uint8_t lo = read_u8(ctx);
    uint8_t hi = read_u8(ctx);
    op->type = Z80_OP_U16_DEREF;
    op->imm16 = (hi << 8) | lo;
}

static void op_reg8(struct disas_ctx* ctx, enum z80_reg8 reg) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R8;
    op->reg8 = reg;
}

static void op_reg16(struct disas_ctx* ctx, enum z80_reg16 reg) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R16;
    op->reg16 = reg;
}

static void op_deref8(struct disas_ctx* ctx, enum z80_reg8 reg) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R8_DEREF;
    op->reg8 = reg;
}

static void op_deref(struct disas_ctx* ctx, enum z80_reg16 reg) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R16_DEREF;
    op->reg16 = reg;
}

static void op_d(struct disas_ctx* ctx) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_JMP_REL;
    op->jmp_disp = 2 + (int16_t) (int8_t) read_u8(ctx);
}

static void op_cc(struct disas_ctx* ctx, uint8_t cc) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_STATUS;
    op->status = cc_tab[cc];
}

static void op_disp(struct disas_ctx* ctx) {
    assert(ctx->is_ix || ctx->is_iy);
    struct z80_op* op = next_operand(ctx);
    op->type = ctx->is_ix ? Z80_OP_IX_REL : Z80_OP_IY_REL;

    if (ctx->is_bit) {
        // Displacement already parsed
        op->displacement = ctx->displacement;
    } else {
        op->displacement = read_u8(ctx);
    }
}

static void op_r(struct disas_ctx* ctx, uint8_t r) {
    switch (r) {
    case 0:
        return op_reg8(ctx, Z80_R8_B);
    case 1:
        return op_reg8(ctx, Z80_R8_C);
    case 2:
        return op_reg8(ctx, Z80_R8_D);
    case 3:
        return op_reg8(ctx, Z80_R8_E);
    case 4:
        if (ctx->is_ix)
            return op_reg8(ctx, Z80_R8_IXH);
        else if (ctx->is_iy)
            return op_reg8(ctx, Z80_R8_IYH);
        return op_reg8(ctx, Z80_R8_H);
    case 5:
        if (ctx->is_ix)
            return op_reg8(ctx, Z80_R8_IXL);
        else if (ctx->is_iy)
            return op_reg8(ctx, Z80_R8_IYL);
        return op_reg8(ctx, Z80_R8_L);
    case 6:
        if (ctx->is_ix || ctx->is_iy)
            return op_disp(ctx);
        return op_deref(ctx, Z80_R16_HL);
    case 7:
        return op_reg8(ctx, Z80_R8_A);
    }
    assert(false);
}

static void op_rp(struct disas_ctx* ctx, uint8_t rp) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R16;
    op->reg16 = rp_tab[rp];
    if (op->reg16 == Z80_R16_HL) {
        if (ctx->is_ix)
            op->reg16 = Z80_R16_IX;
        else if (ctx->is_iy)
            op->reg16 = Z80_R16_IY;
    }
}

static void op_rp2(struct disas_ctx* ctx, uint8_t rp2) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_R16;
    op->reg16 = rp2_tab[rp2];
    if (op->reg16 == Z80_R16_HL) {
        if (ctx->is_ix)
            op->reg16 = Z80_R16_IX;
        else if (ctx->is_iy)
            op->reg16 = Z80_R16_IY;
    }
}

static void op_hl_or_index(struct disas_ctx* ctx) {
    if (ctx->is_ix)
        op_reg16(ctx, Z80_R16_IX);
    else if (ctx->is_iy)
        op_reg16(ctx, Z80_R16_IY);
    else
        op_reg16(ctx, Z80_R16_HL);
}

static enum z80_mnemoric op_alu(struct disas_ctx* ctx, uint8_t alu_op) {
    switch (alu_op) {
    case 0:
        op_reg8(ctx, Z80_R8_A);
        return Z80_ADD;
    case 1:
        op_reg8(ctx, Z80_R8_A);
        return Z80_ADC;
    case 2:
        return Z80_SUB;
    case 3:
        op_reg8(ctx, Z80_R8_A);
        return Z80_SBC;
    case 4:
        return Z80_AND;
    case 5:
        return Z80_XOR;
    case 6:
        return Z80_OR;
    case 7:
        return Z80_CP;
    }
    assert(false);
}

static void op_imm8(struct disas_ctx* ctx, uint8_t imm) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_U8;
    op->imm8 = imm;
}

static void op_im(struct disas_ctx* ctx, uint8_t im) {
    struct z80_op* op = next_operand(ctx);
    op->type = Z80_OP_IM;
    op->im = im_tab[im % 4];
}

static enum z80_mnemoric disassemble_regular(struct disas_ctx* ctx) {
    switch (ctx->opcode.x) {
    case 0:
        switch (ctx->opcode.z) {
        case 0:
            switch (ctx->opcode.y) {
            case 0:
                return Z80_NOP;
            case 1:
                op_reg8(ctx, Z80_R8_A);
                next_operand(ctx)->type = Z80_OP_AF_SHADOW;
                return Z80_EX;
            case 2:
                op_d(ctx);
                return Z80_DJNZ;
            case 3:
                op_d(ctx);
                return Z80_JR;
            default:
                op_cc(ctx, ctx->opcode.y - 4);
                op_d(ctx);
                return Z80_JR;
            }
        case 1:
            switch (ctx->opcode.q) {
            case 0:
                op_rp(ctx, ctx->opcode.p);
                op_nn(ctx);
                return Z80_LD;
            case 1:
                op_hl_or_index(ctx);
                op_rp(ctx, ctx->opcode.p);
                return Z80_ADD;
            }
        case 2:
            switch (ctx->opcode.q) {
            case 0:
                switch (ctx->opcode.p) {
                case 0:
                    op_deref(ctx, Z80_R16_BC);
                    op_reg8(ctx, Z80_R8_A);
                    return Z80_LD;
                case 1:
                    op_deref(ctx, Z80_R16_BC);
                    op_reg8(ctx, Z80_R8_A);
                    return Z80_LD;
                case 2:
                    op_nn_deref(ctx);
                    op_hl_or_index(ctx);
                    return Z80_LD;
                case 3:
                    op_nn_deref(ctx);
                    op_reg8(ctx, Z80_R8_A);
                    return Z80_LD;
                }
            case 1:
                switch (ctx->opcode.p) {
                case 0:
                    op_reg8(ctx, Z80_R8_A);
                    op_deref(ctx, Z80_R16_BC);
                    return Z80_LD;
                case 1:
                    op_reg8(ctx, Z80_R8_A);
                    op_deref(ctx, Z80_R16_DE);
                    return Z80_LD;
                case 2:
                    op_hl_or_index(ctx);
                    op_nn_deref(ctx);
                    return Z80_LD;
                case 3:
                    op_reg8(ctx, Z80_R8_A);
                    op_nn_deref(ctx);
                    return Z80_LD;
                }
            }
        case 3:
            op_rp(ctx, ctx->opcode.p);
            return ctx->opcode.q == 0 ? Z80_INC : Z80_DEC;
        case 4:
            op_r(ctx, ctx->opcode.y);
            return Z80_INC;
        case 5:
            op_r(ctx, ctx->opcode.y);
            return Z80_DEC;
        case 6:
            op_r(ctx, ctx->opcode.y);
            op_n(ctx);
            return Z80_LD;
        case 7:
            switch (ctx->opcode.y) {
            case 0:
                return Z80_RLCA;
            case 1:
                return Z80_RRCA;
            case 2:
                return Z80_RLA;
            case 3:
                return Z80_RRA;
            case 4:
                return Z80_DAA;
            case 5:
                return Z80_CPL;
            case 6:
                return Z80_SCF;
            case 7:
                return Z80_CCF;
            }
        }
    case 1:
        if (ctx->opcode.y == 6 && ctx->opcode.z == 6)
            return Z80_HALT;
        // Special case: if we are parsing an ix or iy instruction, and either of the opcodes is (hl), the other will
        // be the normal variant (h/l) and not the ix variant.
        op_r(ctx, ctx->opcode.y);
        op_r(ctx, ctx->opcode.z);
        if (ctx->opcode.y == 6) {
            if (ctx->opcode.z == 4)
                ctx->inst->operands[1].reg8 = Z80_R8_H;
            else if (ctx->opcode.z == 5)
                ctx->inst->operands[1].reg8 = Z80_R8_L;
        }
        if (ctx->opcode.z == 6) {
            if (ctx->opcode.y == 4)
                ctx->inst->operands[0].reg8 = Z80_R8_H;
            else if (ctx->opcode.y == 5)
                ctx->inst->operands[0].reg8 = Z80_R8_L;
        }
        return Z80_LD;
    case 2: {
            enum z80_mnemoric result = op_alu(ctx, ctx->opcode.y);
            op_r(ctx, ctx->opcode.z);
            return result;
        }
    case 3:
        switch (ctx->opcode.z) {
        case 0:
            op_cc(ctx, ctx->opcode.y);
            return Z80_RET;
        case 1:
            switch (ctx->opcode.q) {
            case 0:
                op_rp2(ctx, ctx->opcode.p);
                return Z80_POP;
            case 1:
                switch (ctx->opcode.p) {
                case 0:
                    return Z80_RET;
                case 1:
                    return Z80_EXX;
                case 2:
                    op_reg16(ctx, Z80_R16_HL);
                    return Z80_JP;
                case 3:
                    op_reg16(ctx, Z80_R16_SP);
                    op_reg16(ctx, Z80_R16_HL);
                    return Z80_LD;
                }
            }
        case 2:
            op_cc(ctx, ctx->opcode.y);
            op_nn(ctx);
            return Z80_JP;
        case 3:
            switch (ctx->opcode.y) {
            case 0:
                op_nn(ctx);
                return Z80_JP;
            case 1:
                // CB prefix
                return Z80_INVALID;
            case 2:
                op_n_deref(ctx);
                op_reg8(ctx, Z80_R8_A);
                return Z80_OUT;
            case 3:
                op_reg8(ctx, Z80_R8_A);
                op_n_deref(ctx);
                return Z80_IN;
            case 4:
                op_deref(ctx, Z80_R16_SP);
                op_hl_or_index(ctx);
                return Z80_EX;
            case 5:
                op_reg16(ctx, Z80_R16_DE);
                op_reg16(ctx, Z80_R16_HL); // Note: never ix/iy
                return Z80_EX;
            case 6:
                return Z80_DI;
            case 7:
                return Z80_EI;
            }
        case 4:
            op_cc(ctx, ctx->opcode.y);
            op_nn(ctx);
            return Z80_CALL;
        case 5:
            switch (ctx->opcode.q) {
            case 0:
                op_rp2(ctx, ctx->opcode.p);
                return Z80_PUSH;
            case 1:
                switch (ctx->opcode.p) {
                case 0:
                    op_nn(ctx);
                    return Z80_CALL;
                case 1:
                    // DD prefix
                case 2:
                    // ED prefix
                case 3:
                    // FD prefix
                    return Z80_INVALID;
                }
            }
        case 6: {
                enum z80_mnemoric result = op_alu(ctx, ctx->opcode.y);
                op_n(ctx);
                return result;
            }
        case 7:
            op_imm8(ctx, ctx->opcode.y * 8);
            return Z80_RST;
        }
    }

    assert(false);
}

static enum z80_mnemoric disassemble_extended(struct disas_ctx* ctx) {
    switch (ctx->opcode.x) {
        case 0:
            return Z80_INVALID;
        case 1:
            switch (ctx->opcode.z) {
            case 0:
                if (ctx->opcode.y != 6)
                    op_r(ctx, ctx->opcode.y);
                op_deref8(ctx, Z80_R8_C);
                return Z80_IN;
            case 1:
                op_deref8(ctx, Z80_R8_C);
                if (ctx->opcode.y != 6)
                    op_r(ctx, ctx->opcode.y);
                else
                    op_imm8(ctx, 0);
                return Z80_OUT;
            case 2:
                op_reg16(ctx, Z80_R16_HL);
                op_rp(ctx, ctx->opcode.p);
                return ctx->opcode.q == 0 ? Z80_SBC : Z80_ADC;
            case 3:
                switch (ctx->opcode.q) {
                case 0:
                    op_nn_deref(ctx);
                    op_rp(ctx, ctx->opcode.p);
                    return Z80_LD;
                case 1:
                    op_rp(ctx, ctx->opcode.p);
                    op_nn_deref(ctx);
                    return Z80_LD;
                }
            case 4:
                return Z80_NEG;
            case 5:
                return ctx->opcode.y == 1 ? Z80_RETI : Z80_RETN;
            case 6:
                op_im(ctx, ctx->opcode.y);
                return Z80_IM;
            }
        case 2:
            switch (ctx->opcode.y) {
            case 0:
                op_reg8(ctx, Z80_R8_I);
                op_reg8(ctx, Z80_R8_A);
                return Z80_LD;
            case 1:
                op_reg8(ctx, Z80_R8_R);
                op_reg8(ctx, Z80_R8_A);
                return Z80_LD;
            case 2:
                op_reg8(ctx, Z80_R8_A);
                op_reg8(ctx, Z80_R8_I);
                return Z80_LD;
            case 3:
                op_reg8(ctx, Z80_R8_A);
                op_reg8(ctx, Z80_R8_I);
                return Z80_LD;
            case 4:
                return Z80_RRD;
            case 5:
                return Z80_RLD;
            case 6:
            case 7:
                return Z80_NOP;
            }
        case 3:
            if (ctx->opcode.z <= 3 && ctx->opcode.y >= 4)
                return bli_tab[ctx->opcode.y - 4][ctx->opcode.z];
            return Z80_INVALID;
    }
    assert(false);
}

static enum z80_mnemoric disassemble_bit(struct disas_ctx* ctx) {
    if (ctx->opcode.x == 0) {
        if (ctx->is_ix || ctx->is_iy) {
            op_disp(ctx);
           if (ctx->opcode.z != 6)
                op_r(ctx, ctx->opcode.z);
           return rot_tab[ctx->opcode.y];
        } else {
            op_r(ctx, ctx->opcode.z);
        }
        return rot_tab[ctx->opcode.y];
    }

    op_imm8(ctx, ctx->opcode.y);
    if (ctx->is_ix || ctx->is_iy) {
        op_disp(ctx);
        if (ctx->opcode.x != 1 && ctx->opcode.z != 6) {
            op_r(ctx, ctx->opcode.z);
        }
    } else {
        op_r(ctx, ctx->opcode.z);
    }

    switch (ctx->opcode.x) {
    case 0:
        assert(false);
    case 1:
        return Z80_BIT;
    case 2:
        return Z80_RES;
    case 3:
        return Z80_SET;
    }
    assert(false);
}

void z80_disassemble(struct z80_inst* inst, size_t len, const uint8_t code[len]) {
    struct disas_ctx ctx = {
        .inst = inst,
        .code_len = len,
        .code = code,
    };

    inst->mnemoric = Z80_INVALID;
    for (size_t i = 0; i < Z80_MAX_OPERANDS; ++i)
        inst->operands[i].type = Z80_OP_NONE;
    inst->size = 0;

    uint8_t opcode = read_u8(&ctx);
    ctx.is_ix = opcode == 0xDD;
    ctx.is_iy = opcode == 0xDF;
    ctx.is_extended = opcode == 0xED;
    ctx.is_bit = opcode == 0xCB;

    if (ctx.is_ix || ctx.is_iy) {
        opcode = read_u8(&ctx);
        ctx.is_bit = opcode == 0xCB;
        if (ctx.is_bit) {
            // Second instruction form: two prefix bytes, displacement byte, opcode
            ctx.displacement = read_u8(&ctx);
            opcode = read_u8(&ctx);
        }
    } else if (ctx.is_extended || ctx.is_bit) {
        opcode = read_u8(&ctx);
    }

    ctx.opcode.full = opcode;
    if (ctx.out_of_bounds)
        return;

    if (ctx.is_extended) {
        inst->mnemoric = disassemble_extended(&ctx);
    } else if (ctx.is_bit) {
        inst->mnemoric = disassemble_bit(&ctx);
    } else {
        inst->mnemoric = disassemble_regular(&ctx);
    }

    if (ctx.out_of_bounds) {
        inst->mnemoric = Z80_INVALID;
        for (size_t i = 0; i < Z80_MAX_OPERANDS; ++i)
            inst->operands[i].type = Z80_OP_NONE;
    }
}
