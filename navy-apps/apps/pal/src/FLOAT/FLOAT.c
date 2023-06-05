#include "FLOAT.h"
#include <assert.h>
#include <stdint.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
    // assert(0);
    // return 0;
    return (a * b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    int sign = (a < 0 ? -1 : 1) * (b < 0 ? -1 : 1);
    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    int ret = a / b;
    unsigned r = a % b;
    int i = 0;
    for (i = 0; i < 16; i++) {
        ret *= 2;
        r *= 2;
        while (r >= b)
            ret++, r -= b;
    }
    return ret * sign;
    // assert(0);
    // return 0;
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
    union float_ {
        struct {
            uint32_t mantissa : 23;
            uint32_t exp : 8;
            uint32_t sign : 1;
        };
        uint32_t value;
    };
    union float_ f;
    f.value = *((uint32_t *)(void *)&a);

    int e = f.exp - 127;

    FLOAT result;
    if (e <= 7) {
        result = (f.mantissa | (1 << 23)) >> 7 - e;
    } else {
        result = (f.mantissa | (1 << 23)) << (e - 7);
    }
    return f.sign == 0 ? result : (result | (1 << 31));
}

FLOAT Fabs(FLOAT a) {
    assert(0);
    return 0;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
    FLOAT dt, t = int2F(2);

    do {
        dt = F_div_int((F_div_F(x, t) - t), 2);
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
    /* we only compute x^0.333 */
    FLOAT t2, dt, t = int2F(2);

    do {
        t2 = F_mul_F(t, t);
        dt = (F_div_F(x, t2) - t) / 3;
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}
