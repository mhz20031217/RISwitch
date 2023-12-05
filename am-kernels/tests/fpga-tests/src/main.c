#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void delay() {
  for (volatile int i = 0; i < 100000; i ++);
}

void led_test() {
  for (uint16_t i = 1; i; i <<= 1) {
    io_write(AM_LED, i);
    // delay();
  }
}

void seg_test() {
  uint32_t v = 0xabcdef88;
  for (int i = 0; i < 20; i ++) {
    io_write(AM_SEG, v);
    // delay();
    i = (i << 4) | (i >> 28);
  }
}

int main(const char *args) {
  ioe_init();

  led_test();  
  seg_test();
  halt(SWITCH_EXIT_SUCCESS);
}
