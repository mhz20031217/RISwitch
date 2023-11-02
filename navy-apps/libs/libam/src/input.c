#include "amdev.h"
#include <am.h>
#include <NDL.h>
#include <string.h>

#define keyname(k) #k,
#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)


static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
#define ARRLEN(x) (sizeof(x)/sizeof((x)[0]))
#define NR_KEYS ARRLEN(keyname)


void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  char buf[64];
  int rc = NDL_PollEvent(buf, sizeof(buf) - 1);

  if (rc) {
    if (buf[0] != 'k') {
      printf("[libam] spec error: event not beginning with 'k'.\n");
      goto error;
    }
    for (int i = 0; i < NR_KEYS; i ++) {
      if (strcmp(buf + 3, keyname[i]) == 0) {
        kbd->keycode = i;
        break;
      }
    }
    if (buf[1] == 'd') {
      kbd->keydown = true;
    } else if (buf[1] == 'u') {
      kbd->keydown = false;
    } else {
      goto error;
    }
    return;
  }

error: // fall through
  kbd->keydown = false;
  kbd->keycode = AM_KEY_NONE;
}
