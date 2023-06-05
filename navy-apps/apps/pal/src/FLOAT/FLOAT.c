// include "FLOAT.h"
// include <assert.h>
// include <stdint.h>

struct float_ {
    uint32_t mantissa : 23; // 定义23位的尾数字段，表示浮点数的小数部分
    uint32_t exp : 8; // 定义8位的指数字段，表示浮点数的指数部分
    uint32_t sign : 1; // 定义1位的符号字段，表示浮点数的正负号
};

FLOAT F_mul_F(FLOAT a, FLOAT b) {
    // assert(0);
    // return 0;
    return ((int64_t)a * (int64_t)b) >> 16;
    // return (a * b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    FLOAT dividend = Fabs(a);
    FLOAT divisor = Fabs(b);
    // 执行浮点数的除法操作，得到初始结果
    FLOAT res = dividend / divisor;
    dividend = dividend % divisor;

    // 长除法
    for (int i = 0; i < 16; i++) {
        dividend <<= 1;  // 将被除数左移1位
        res <<= 1;  // 将结果左移1位
        // 如果被除数大于等于除数
        if (dividend >= divisor) { 
            dividend -= divisor;
            res++;
        }
    }
    // 如果a和b的符号不同
    if (((a ^ b) & 0x80000000) == 0x80000000) {
        res = -res;
    }
    return res;
}


FLOAT f2F(float a) {
    /* You should figure out how to convert `a' into FLOAT without
     * introducing x87 floating point instructions. Else you can
     * not run this code in NEMU before implementing x87 floating
     * point instructions, which is contrary to our expectation.
     *
     * Hint: The bit representation of `a' is already on the
     * stack. How do you retrieve it to another variable without
     * performing arithmetic operations on it directly?
     */

    struct float_ *f = (struct float_ *)&a;
    uint32_t res;      // 用于存储结果的变量
    uint32_t mantissa; // 存储提取后的尾数
    int exp;           // 存储提取后的指数
    if ((f->exp & 0xff) == 0xff) // 如果指数字段全为1（特殊值NaN或无穷大）
        assert(0);
    else if (f->exp == 0) { // 如果指数字段为0（非规格化数）
        exp = 1 - 127;
        mantissa = (f->mantissa & 0x7fffff); // 提取尾数字段（除去隐藏的1）
    } else {
        exp = f->exp - 127; // 提取指数字段（减去偏置值127）
        // 提取尾数字段，并将隐藏的1添加到尾数的最高位
        mantissa = (f->mantissa & 0x7fffff) | (1 << 23);
    }
    if (exp >= 7 && exp < 22) // 如果结果可以表示为规格化的浮点数
        res = mantissa << (exp - 7); // 左移尾数
    else if (exp < 7 && exp > -32) // 如果结果可以表示为非规格化的浮点数）
        res = mantissa >> 7 >> -exp; // 右移尾数
    return (f->sign) ? -res : res;
}

FLOAT Fabs(FLOAT a) { return (a > 0) ? a : -a; }

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT dividend) {
    FLOAT dt, t = int2F(2);

    do {
        dt = F_div_int((F_div_F(dividend, t) - t), 2);
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}

FLOAT Fpow(FLOAT dividend, FLOAT divisor) {
    /* we only compute dividend^0.333 */
    FLOAT t2, dt, t = int2F(2);

    do {
        t2 = F_mul_F(t, t);
        dt = (F_div_F(dividend, t2) - t) / 3;
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}
