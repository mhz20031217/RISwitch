#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void delay() {
  for (volatile int i = 0; i < 100000; i ++);
}

void led_test() {
  for (volatile uint16_t i = 1; i; i = i * 2) {
    io_write(AM_LED, i);
    // delay();
  }
  io_write(AM_LED, 1234);
}

void seg_test() {
  volatile uint32_t v = 0xabcdef88;
  for (volatile int i = 0; i < 100; i ++) {
    io_write(AM_SEG, v);
    // delay();
    v = (v << 4) | (v >> 28);
  }
}

int main(const char *args) {
  ioe_init();

  led_test();  
  seg_test();
  halt(SWITCH_EXIT_SUCCESS);
}
