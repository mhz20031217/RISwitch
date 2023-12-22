#include "amdev.h"
#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W 70
#define H 30

typedef void (*handler_t)(int *args);

struct Pattern {
  const char *pattern;
  handler_t handle;
};

void move_one();
void backspace();
size_t write_escape(const char *str, size_t count);
void scroll_up();

void esc_move(int *args);
void esc_movefirst(int *args);
void esc_moveup(int *args);
void esc_movedown(int *args);
void esc_moveleft(int *args);
void esc_moveright(int *args);
void esc_save(int *args);
void esc_restore(int *args);
void esc_clear(int *args);
void esc_erase(int *args);
void esc_setattr1(int *args);
void esc_setattr2(int *args);
void esc_setattr3(int *args);
void esc_rawmode(int *args);
void esc_cookmode(int *args);

void init_terminal(int width, int height);
void destroy_terminal();
void write(const char *str, size_t count);
bool is_dirty(int x, int y);

void putcha(int x, int y, char ch);
char getch(int x, int y);
enum Color foreground(int x, int y); // get color
enum Color background(int x, int y);

void clear(); // clear dirty states
const char *keypress(char ch);

enum Mode {
  raw,
  cook,
} mode;

struct Pattern esc_seqs[];

char *buf, input[256], cooked[256];
uint8_t *color;
bool *dirty;
int inp_len;

int w, h;
struct Cursor {
  int x, y;
} cursor, saved;
uint8_t col_f, col_b;

void refresh_terminal();

#define EMPTY ' '

enum Color {
  BLACK = 0,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
};

struct Pattern esc_seqs[] = {
    {"\033[1t", esc_cookmode}, // added by us
    {"\033[2t", esc_rawmode},  // added by us

    {"\033[s", esc_save},        {"\033[u", esc_restore},
    {"\033[J", esc_clear},       {"\033[2J", esc_clear},
    {"\033[K", esc_erase},       {"\033[f", esc_movefirst},
    {"\033[H", esc_movefirst},   {"\033[#;#f", esc_move},
    {"\033[#;#H", esc_move},     {"\033[#A", esc_moveup},
    {"\033[#B", esc_movedown},   {"\033[#C", esc_moveright},
    {"\033[#D", esc_moveleft},   {"\033[#m", esc_setattr1},
    {"\033[#;#m", esc_setattr2}, {"\033[#;#;#m", esc_setattr3},
};

inline int min(int x, int y) { return x < y ? x : y; }
inline int max(int x, int y) { return x > y ? x : y; }

void esc_move(int *args) {
  cursor.x = args[1] - 1;
  cursor.y = args[0] - 1;
}

void esc_movefirst(int *args) {
  cursor.x = 0;
  cursor.y = 0;
}

void esc_moveup(int *args) { cursor.y -= args[0]; }

void esc_movedown(int *args) { cursor.y += args[0]; }

void esc_moveleft(int *args) { cursor.x -= args[0]; }

void esc_moveright(int *args) { cursor.x += args[0]; }

void esc_save(int *args) { saved = cursor; }

void esc_restore(int *args) { cursor = saved; }

void esc_clear(int *args) {
  for (int i = 0; i < w; i++)
    for (int j = 0; j < h; j++) {
      putcha(i, j, EMPTY);
    }
  cursor.x = 0;
  cursor.y = 0;
}

void esc_setattr1(int *args) {
  int attr = args[0];
  switch (attr) {
  case 0:
    col_f = BLACK;
    col_b = WHITE;
    break; // reset
  case 30:
    col_f = BLACK;
    break;
  case 31:
    col_f = RED;
    break;
  case 32:
    col_f = GREEN;
    break;
  case 33:
    col_f = YELLOW;
    break;
  case 34:
    col_f = BLUE;
    break;
  case 35:
    col_f = MAGENTA;
    break;
  case 36:
    col_f = CYAN;
    break;
  case 37:
    col_f = WHITE;
    break;
  case 40:
    col_b = BLACK;
    break;
  case 41:
    col_b = RED;
    break;
  case 42:
    col_b = GREEN;
    break;
  case 43:
    col_b = YELLOW;
    break;
  case 44:
    col_b = BLUE;
    break;
  case 45:
    col_b = MAGENTA;
    break;
  case 46:
    col_b = CYAN;
    break;
  case 47:
    col_b = WHITE;
    break;
  }
}

void esc_setattr2(int *args) {
  esc_setattr1(&args[0]);
  esc_setattr1(&args[1]);
}

void esc_setattr3(int *args) {
  esc_setattr1(&args[0]);
  esc_setattr1(&args[1]);
  esc_setattr1(&args[2]);
}

void esc_erase(int *args) {
  for (int i = cursor.x; i < w; i++) {
    putcha(i, cursor.y, EMPTY);
  }
}

void esc_rawmode(int *args) { mode = raw; }

void esc_cookmode(int *args) { mode = cook; }

void init_terminal(int width, int height) {
  w = width;
  h = height;
  mode = cook;
  cursor.x = 0;
  cursor.y = 0;
  saved = cursor;
  buf = malloc(w * h * sizeof(char));
  color = malloc(w * h * sizeof(uint8_t));
  dirty = malloc(w * h * sizeof(bool));
  inp_len = 0;
  col_f = BLACK;
  col_b = WHITE;
  input[0] = '\0';

  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      putcha(x, y, EMPTY);
    }
  }
  printf("Terminal initialized!\n");
}

void destroy_terminal() {
  free(buf);
  free(color);
  free(dirty);
}

void backspace() {
  cursor.x--;
  if (cursor.x < 0) {
    cursor.x = w - 1;
    cursor.y--;
    if (cursor.y < 0)
      cursor.x = cursor.y = 0;
  }
  buf[cursor.y * w + cursor.x] = EMPTY;
  dirty[cursor.y * w + cursor.x] = true;
}

void move_one() {
  int ret = cursor.y * w + cursor.x;
  cursor.x++;
  if (cursor.x >= w) {
    cursor.x = 0;
    cursor.y++;
  }
  if (cursor.y >= h) {
    scroll_up();
    ret -= w;
    cursor.y--;
  }
}

void scroll_up() {
  memmove(buf, buf + w, w * (h - 1));
  memmove(color, color + w, w * (h - 1));
  for (int i = 0; i < w; i++) {
    putcha(i, h - 1, EMPTY);
  }
  for (int i = 0; i < w * h; i++) {
    dirty[i] = true;
  }
}

size_t write_escape(const char *str, size_t count) {
  for (struct Pattern *p = esc_seqs; p < esc_seqs + 18; p++) {
    bool match = false;
    int len = 0, args[4], narg = 0;

    for (const char *cur = p->pattern, *s = str;; cur++) {
      if (*cur == '\0') { // found a match.
        match = true;
        break;
      }
      if (*cur != '#') {
        if (*s != *cur)
          break;
        s++;
        len++;
      } else {
        int data = 0;
        for (; *s >= '0' && *s <= '9' && s - str < count; s++, len++) {
          data = data * 10 + *s - '0';
        }
        args[narg++] = data;
      }
    }

    if (match) {
      (p->handle)(args);
      cursor.x = max(min(cursor.x, w - 1), 0);
      cursor.y = max(min(cursor.y, h - 1), 0);
      return len;
    }
  }
  return 1;
}

void write(const char *str, size_t count) {
  for (size_t i = 0; i != count && str[i];) {
    char ch = str[i];
    if (ch == '\033') {
      i += write_escape(&str[i], count - i);
    } else {
      switch (ch) {
      case '\x07':
        break;
      case '\n':
        cursor.x = 0;
        cursor.y++;
        if (cursor.y >= h) {
          scroll_up();
          cursor.y--;
        }
        break;
      case '\t':
        // TODO: implement it.
        break;
      case '\r':
        cursor.x = 0;
        break;
      default:
        putcha(cursor.x, cursor.y, ch);
        move_one();
      }
      i++;
    }
  }
}

const char *keypress(char ch) {
  if (ch == '\0')
    return NULL;
  if (mode == raw) {
    input[0] = ch;
    input[1] = '\0';
    return input;
  } else if (mode == cook) {
    const char *ret = NULL;
    switch (ch) {
    case '\033':
      break;
    case '\n':
      strcpy(cooked, input);
      strcat(cooked, "\n");
      ret = cooked;
      write("\n", 1);
      inp_len = 0;
      break;
    case '\b':
      if (inp_len > 0) {
        inp_len--;
        backspace();
      }
      break;
    default:
      if (inp_len + 1 < sizeof(input)) {
        input[inp_len++] = ch;
        write(&ch, 1);
      }
    }
    input[inp_len] = '\0';
    return ret;
  }
  return NULL;
}

char getch(int x, int y) { return buf[x + y * w]; }

void putcha(int x, int y, char ch) {
  buf[x + y * w] = ch;
  color[x + y * w] = (col_f << 4) | col_b;
  dirty[x + y * w] = true;
}

enum Color foreground(int x, int y) { return color[x + y * w]; }

enum Color background(int x, int y) { return color[x + y * w] & 0xf; }

bool is_dirty(int x, int y) { return dirty[x + y * w]; }

void clear() {
  for (int i = 0; i < w * h; i++)
    dirty[i] = false;
  dirty[cursor.x + cursor.y * w] = true;
}

char screen[W][H];
uint8_t fg_color[W][H], bg_color[W][H];

void draw_ch(int x, int y, char ch, enum Color fg, enum Color bg) {
  printf("Draw char '%c' at (%d, %d) with fg = %d, bg = %d\n", ch, x, y, fg, bg);
  io_write(AM_CMEM_PUTCH, x, y, ch, fg, bg);
}

void refresh_terminal() {
  for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++)
      if (is_dirty(i, j)) {
        draw_ch(i, j, getch(i, j), foreground(i, j), background(i, j));
      }
  clear();

  // uint64_t last = 0;
  // int flip = 0;
  // uint64_t now = io_read(AM_TIMER_UPTIME).us;
  // if (now - last > 500000 || needsync) {
  //   int x = cursor.x, y = cursor.y;
  //   uint32_t color = (flip ? foreground(x, y) : background(x, y));
  //   draw_ch(x, y, ' ', 0, color);
  //   for (int i = 0; i < W; i++) {
  //     for (int j = 0; j < H; j++) {
  //       io_write(AM_CMEM_PUTCH, i, j, screen[i][j], fg_color[i][j],
  //                bg_color[i][j]);
  //     }
  //   }
  //   if (now - last > 500000) {
  //     flip = !flip;
  //     last = now;
  //   }
  // }
}

#define ENTRY(KEYNAME, NOSHIFT, SHIFT) [AM_KEY_##KEYNAME] = {NOSHIFT, SHIFT}
const struct ShiftPattern {
  char noshift, shift;
} SHIFT[256] = {
    ENTRY(ESCAPE, '\033', '\033'),
    ENTRY(SPACE, ' ', ' '),
    ENTRY(RETURN, '\n', '\n'),
    ENTRY(BACKSPACE, '\b', '\b'),
    ENTRY(1, '1', '!'),
    ENTRY(2, '2', '@'),
    ENTRY(3, '3', '#'),
    ENTRY(4, '4', '$'),
    ENTRY(5, '5', '%'),
    ENTRY(6, '6', '^'),
    ENTRY(7, '7', '&'),
    ENTRY(8, '8', '*'),
    ENTRY(9, '9', '('),
    ENTRY(0, '0', ')'),
    ENTRY(GRAVE, '`', '~'),
    ENTRY(MINUS, '-', '_'),
    ENTRY(EQUALS, '=', '+'),
    ENTRY(SEMICOLON, ';', ':'),
    ENTRY(APOSTROPHE, '\'', '"'),
    ENTRY(LEFTBRACKET, '[', '{'),
    ENTRY(RIGHTBRACKET, ']', '}'),
    ENTRY(BACKSLASH, '\\', '|'),
    ENTRY(COMMA, ',', '<'),
    ENTRY(PERIOD, '.', '>'),
    ENTRY(SLASH, '/', '?'),
    ENTRY(A, 'a', 'A'),
    ENTRY(B, 'b', 'B'),
    ENTRY(C, 'c', 'C'),
    ENTRY(D, 'd', 'D'),
    ENTRY(E, 'e', 'E'),
    ENTRY(F, 'f', 'F'),
    ENTRY(G, 'g', 'G'),
    ENTRY(H, 'h', 'H'),
    ENTRY(I, 'i', 'I'),
    ENTRY(J, 'j', 'J'),
    ENTRY(K, 'k', 'K'),
    ENTRY(L, 'l', 'L'),
    ENTRY(M, 'm', 'M'),
    ENTRY(N, 'n', 'N'),
    ENTRY(O, 'o', 'O'),
    ENTRY(P, 'p', 'P'),
    ENTRY(Q, 'q', 'Q'),
    ENTRY(R, 'r', 'R'),
    ENTRY(S, 's', 'S'),
    ENTRY(T, 't', 'T'),
    ENTRY(U, 'u', 'U'),
    ENTRY(V, 'v', 'V'),
    ENTRY(W, 'w', 'W'),
    ENTRY(X, 'x', 'X'),
    ENTRY(Y, 'y', 'Y'),
    ENTRY(Z, 'z', 'Z'),
};

char handle_key(AM_INPUT_KEYBRD_T *k) {
  static int shift = 0;

  if (k->keycode == AM_KEY_LSHIFT || k->keycode == AM_KEY_RSHIFT) {
    shift ^= 1;
    return '\0';
  }

  if (k->keydown) {
    const struct ShiftPattern *item = &SHIFT[k->keycode];
    if (shift)
      return item->shift;
    else
      return item->noshift;
  }
  return '\0';
}

void sh_printf(const char *format, ...) {
  char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  write(buf, len);
  printf("sh_printf(%s)\n", buf);
}

void sh_banner() { sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n"); }

void sh_prompt() { sh_printf("sh> "); }

uint64_t fib(int n) {
  if (n < 0) return 0;
  uint64_t t1 = 0, t2 = 1, now = 1;
  if (n == 0) return t1;
  else if (n == 1) return t2;
  else if (n == 3) return now;
  while (n > 3) {
    t1 = t2; t2 = now; now = t1 + t2;
    n --;
  }
  return now;
}

static void sh_handle_cmd(const char *command) {
  sh_printf("Hello, world!\n");
}


void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    AM_INPUT_KEYBRD_T key = io_read(AM_INPUT_KEYBRD);
    if (key.keycode == AM_KEY_NONE) continue;
    const char *res = keypress(handle_key(&key));
    if (res) {
      sh_handle_cmd(res);
      sh_prompt();
    }
    refresh_terminal();
  }
}

int main(const char *args) {
  ioe_init();
  init_terminal(W, H);

  builtin_sh_run();

  return 1;
}
