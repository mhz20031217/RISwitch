#include <fixedptc.h>
#include <stdio.h>

fixedpt fixedpt_fromfloat(void *p) {
    uint32_t o = *(uint32_t *)p;
    if (o == 0U || o == (1U << 31)) {
        return (fixedpt) 0U;
    }
    fixedpt r = 0;

    uint32_t s, f, e;
    s = (o & (1U << 31)) ? 1 : 0;
    f = (o & ((1U << 23) - 1));
    e = (o >> 23) & ((1U << 11) - 1);

    int32_t t = f | (1U << 23);
    if (s) {
        t = ~t + 1;
    }

}

int main() {
    printf(
        "[libfixedptc] Test: 123.456 * 456.789 = %s.\n",
        fixedpt_cstr(
            fixedpt_mul(fixedpt_rconst(123.456), fixedpt_rconst(456.789)), 8));
    printf("[libfixedptc] Test: floor(123.456) = %s.\n",
      fixedpt_cstr(fixedpt_floor(fixedpt_rconst(123.456)), 8));
}
