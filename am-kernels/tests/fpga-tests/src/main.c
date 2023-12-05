#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void delay() {
  for (volatile int i = 0; i < 100; i ++);
}

void led_test() {
  for (uint16_t i = 1; i; i <<= 1) {
    io_write(AM_LED, i);
    //delay();
  }
}

void seg_test() {
  uint32_t v = 0xabcdef88;
  for (int i = 0; i < 100000000; i ++) {
    io_write(AM_SEG, v);
    delay();
    v = (v << 4) | (v >> 28);
  }
}

int main(const char *args) {
  ioe_init();

  led_test();  
  seg_test();
  halt(SWITCH_EXIT_SUCCESS);
}
