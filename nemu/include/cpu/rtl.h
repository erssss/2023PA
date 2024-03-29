#ifndef __RTL_H__
#define __RTL_H__

#include "nemu.h"

extern rtlreg_t t0, t1, t2, t3;
extern const rtlreg_t tzero;

/* RTL basic instructions */
// RTL 基本指令
static inline void rtl_li(rtlreg_t *dst, uint32_t imm) { *dst = imm; }

#define c_add(a, b) ((a) + (b))
#define c_sub(a, b) ((a) - (b))
#define c_and(a, b) ((a) & (b))
#define c_or(a, b) ((a) | (b))
#define c_xor(a, b) ((a) ^ (b))
#define c_shl(a, b) ((a) << (b))
#define c_shr(a, b) ((a) >> (b))
#define c_sar(a, b) ((int32_t)(a) >> (b))
#define c_slt(a, b) ((int32_t)(a) < (int32_t)(b))
#define c_sltu(a, b) ((a) < (b))

#define make_rtl_arith_logic(name)                                             \
    static inline void concat(rtl_, name)(                                     \
        rtlreg_t * dst, const rtlreg_t *src1, const rtlreg_t *src2) {         \
        (*dst) = concat(c_, name)(*src1, *src2);                              \
    }                                                                          \
    static inline void concat3(rtl_, name, i)(rtlreg_t * dst,                 \
                                              const rtlreg_t *src1, int imm) { \
        (*dst) = concat(c_, name)(*src1, imm);                                \
    }

make_rtl_arith_logic(add) make_rtl_arith_logic(sub) make_rtl_arith_logic(and)
    make_rtl_arith_logic(or) make_rtl_arith_logic(xor) make_rtl_arith_logic(shl)
        make_rtl_arith_logic(shr) make_rtl_arith_logic(sar)
            make_rtl_arith_logic(slt) make_rtl_arith_logic(sltu)

                static inline void rtl_mul(rtlreg_t *dest_hi, rtlreg_t *dest_lo,
                                           const rtlreg_t *src1,
                                           const rtlreg_t *src2) {
    asm volatile("mul %3"
                 : "=d"(*dest_hi), "=a"(*dest_lo)
                 : "a"(*src1), "r"(*src2));
}

static inline void rtl_imul(rtlreg_t *dest_hi, rtlreg_t *dest_lo,
                            const rtlreg_t *src1, const rtlreg_t *src2) {
    asm volatile("imul %3"
                 : "=d"(*dest_hi), "=a"(*dest_lo)
                 : "a"(*src1), "r"(*src2));
}

static inline void rtl_div(rtlreg_t *q, rtlreg_t *r, const rtlreg_t *src1_hi,
                           const rtlreg_t *src1_lo, const rtlreg_t *src2) {
    asm volatile("div %4"
                 : "=a"(*q), "=d"(*r)
                 : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_idiv(rtlreg_t *q, rtlreg_t *r, const rtlreg_t *src1_hi,
                            const rtlreg_t *src1_lo, const rtlreg_t *src2) {
    asm volatile("idiv %4"
                 : "=a"(*q), "=d"(*r)
                 : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_lm(rtlreg_t *dst, const rtlreg_t *addr, int len) {
    *dst = vaddr_read(*addr, len);
}

static inline void rtl_sm(rtlreg_t *addr, int len, const rtlreg_t *src1) {
    vaddr_write(*addr, len, *src1);
}

static inline void rtl_lr_b(rtlreg_t *dst, int r) { *dst = reg_b(r); }

static inline void rtl_lr_w(rtlreg_t *dst, int r) { *dst = reg_w(r); }

static inline void rtl_lr_l(rtlreg_t *dst, int r) { *dst = reg_l(r); }

static inline void rtl_sr_b(int r, const rtlreg_t *src1) { reg_b(r) = *src1; }

static inline void rtl_sr_w(int r, const rtlreg_t *src1) { reg_w(r) = *src1; }

static inline void rtl_sr_l(int r, const rtlreg_t *src1) { reg_l(r) = *src1; }

/* RTL psuedo instructions */

static inline void rtl_lr(rtlreg_t *dst, int r, int width) {
    switch (width) {
    case 4:
        rtl_lr_l(dst, r);
        return;
    case 1:
        rtl_lr_b(dst, r);
        return;
    case 2:
        rtl_lr_w(dst, r);
        return;
    default:
        assert(0);
    }
}

static inline void rtl_sr(int r, int width, const rtlreg_t *src1) {
    switch (width) {
    case 4:
        rtl_sr_l(r, src1);
        return;
    case 1:
        rtl_sr_b(r, src1);
        return;
    case 2:
        rtl_sr_w(r, src1);
        return;
    default:
        assert(0);
    }
}

#define make_rtl_setget_eflags(f)                                              \
    static inline void concat(rtl_set_, f)(const rtlreg_t *src) {              \
        cpu.eflags.f = *src;                                                   \
    }                                                                          \
    static inline void concat(rtl_get_, f)(rtlreg_t * dst) {                  \
        *dst = cpu.eflags.f;                                                  \
    }

make_rtl_setget_eflags(CF) make_rtl_setget_eflags(OF) make_rtl_setget_eflags(ZF)
    make_rtl_setget_eflags(SF)

        static inline void rtl_mv(rtlreg_t *dst, const rtlreg_t *src1) {
    // dst <- src1
    // TODO();
    rtl_addi(dst, src1, 0);
}

static inline void rtl_not(rtlreg_t *dst) {
    // dst <- ~dst
    // TODO();
    rtl_xori(dst, dst, 0xffffffff);
}

static inline void rtl_sext(rtlreg_t *dst, const rtlreg_t *src1, int width) {
    // dst <- signext(src1[(width * 8 - 1) .. 0])
    // TODO();
    if (width == 4)
        rtl_mv(dst, src1);
    else {
        assert(width == 1 || width == 2);
        rtl_shli(dst, src1, (4 - width) * 8);
        rtl_sari(dst, dst, (4 - width) * 8);
    }
}

static inline void rtl_push(const rtlreg_t *src1) {
    // esp <- esp - 4
    // M[esp] <- src1
    // TODO();
    rtl_subi(&cpu.esp, &cpu.esp, 4);
    rtl_sm(&cpu.esp, 4, src1);
}

static inline void rtl_pop(rtlreg_t *dst) {
    // dst <- M[esp]
    // esp <- esp + 4
    // TODO();
    rtl_lm(dst, &cpu.esp, 4);
    rtl_addi(&cpu.esp, &cpu.esp, 4);
}

// 更新eflags的各种标志位
// 判断src是否为0
static inline void rtl_eq0(rtlreg_t *dst, const rtlreg_t *src1) {
    // dst <- (src1 == 0 ? 1 : 0)
    // TODO();
    rtl_sltui(dst, src1, 1); // 如果src小于1（即=0），则dest置为1
}

// 判断src和imm是否相等，即判断它俩异或是否为0
static inline void rtl_eqi(rtlreg_t *dst, const rtlreg_t *src1, int imm) {
    // dst <- (src1 == imm ? 1 : 0)
    // TODO();
    rtl_xori(dst, src1, imm); // dst=src^imm
    rtl_eq0(dst, dst);
}

// 如果src不为0，返回1
static inline void rtl_neq0(rtlreg_t *dst, const rtlreg_t *src1) {
    // dst <- (src1 != 0 ? 1 : 0)
    // TODO();
    rtl_eq0(dst, src1);
    rtl_eq0(dst, dst);
}

// 获得src1[width * 8 - 1]这一位
static inline void rtl_msb(rtlreg_t *dst, const rtlreg_t *src1, int width) {
    // dst <- src1[width * 8 - 1]
    // TODO();
    rtl_shri(dst, src1, width * 8 - 1); // 右移，直到最高位是需要的位
    rtl_andi(dst, dst, 0x1);
}

static inline void rtl_update_ZF(const rtlreg_t *result, int width) {
    // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
    // TODO();
    rtl_andi(&t0, result,
             (0xffffffffu >> (4 - width) * 8)); // 只获取result的后width字节
    rtl_eq0(&t0, &t0);
    rtl_set_ZF(&t0);
}

// 符号位
static inline void rtl_update_SF(const rtlreg_t *result, int width) {
    // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
    // TODO();
    rtl_msb(&t0, result, width);
    rtl_set_SF(&t0);
}

static inline void rtl_update_ZFSF(const rtlreg_t *result, int width) {
    rtl_update_ZF(result, width);
    rtl_update_SF(result, width);
}

// cr寄存器的读写rtl指令

static inline void rtl_load_cr(rtlreg_t *dst, int r) {
    switch (r) {
    case 0:
        *dst = cpu.CR0;
        return;
    case 3:
        *dst = cpu.CR3;
        return;
    default:
        assert(0);
    }
    return;
}
static inline void rtl_store_cr(int r, const rtlreg_t *src) {
    switch (r) {
    case 0:
        cpu.CR0 = *src;
        return;
    case 3:
        cpu.CR3 = *src;
        return;
    default:
        assert(0);
    }
    return;
}

#endif
