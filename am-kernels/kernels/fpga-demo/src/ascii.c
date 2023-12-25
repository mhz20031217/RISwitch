#include <amdev.h>

#define K(am, ascii)                                                           \
  case AM_KEY_##am:                                                            \
    return ascii;

#define D(c) K(c, (#c)[0])

#define ALNUMS(_) \
  _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) \
  _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) \
  _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) \
  _(Z) _(X) _(C) _(V) _(B) _(N) _(M)

char to_ascii(int am_keycode) {
  switch (am_keycode) {
    ALNUMS(D)
  default:
    return '\0';
  }
}
