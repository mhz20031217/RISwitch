#include <fixedptc.h>
#include <stdio.h>

int main() {
    printf(
        "[libfixedptc] Test: 123.456 * 456.789 = %s.\n",
        fixedpt_cstr(
            fixedpt_mul(fixedpt_rconst(123.456), fixedpt_rconst(456.789)), 8));
    printf("[libfixedptc] Test: floor(123.456) = %s.\n",
      fixedpt_cstr(fixedpt_floor(fixedpt_rconst(123.456)), 8));
}
