#include "amdev.h"
#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void delay() {
  for (volatile int i = 0; i < 10000; i ++);
}

void led_test() {
  for (volatile uint16_t i = 1; i; i = i * 2) {
    io_write(AM_LED, i);
    delay();
  }
  io_write(AM_LED, 1234);
}

void seg_test() {
  volatile uint32_t v = 0xabcdef88;
  for (volatile int i = 0; i < 100; i ++) {
    io_write(AM_SEG, v);
    delay();
    v = (v << 4) | (v >> 28);
  }
}

void cmem_test() {
  AM_CMEM_CONFIG_T c = io_read(AM_CMEM_CONFIG);

  for (int i = 0; i < c.width; i ++) {
    for (int j = 0; j < c.height; j ++) {
      io_write(AM_CMEM_PUTCH, i, j, rand() % 26 + 'a', rand() % 8, rand() % 8);
    }
  }

  // while (true);
}

int main(const char *args) {
  ioe_init();

  // led_test();  
  // seg_test();

  cmem_test();
  halt(SWITCH_EXIT_SUCCESS);
}
