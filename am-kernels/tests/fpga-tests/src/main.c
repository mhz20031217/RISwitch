#include <am.h>
#include <klib.h>
#include <klib-macros.h>

void delay() {
  uint64_t start = io_read(AM_TIMER_UPTIME).us;

  // 1 second
  while (io_read(AM_TIMER_UPTIME).us - start < 10);
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
  for (volatile int i = 0; i < 10; i ++) {
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

  // char *hello = "hello, world!";
  // int len = strlen(hello);
  // for (int i = 0; i < len; i ++) {
  //   io_write(AM_CMEM_PUTCH, i, 0, hello[i], 0, 7);
  // }
}

void serial_test() {
  char *hello = "hello, world!\n";
  int len = strlen(hello);
  for (int i = 0; i < len; i ++) {
    io_write(AM_SERIAL_PUTCH, hello[i]);
  }
  printf("%s", hello);
}

void timer_test() {
  // printf("Timer test begin.\n");

  for (int i = 0; i < 10; i ++) {
    printf("%llu\n", io_read(AM_TIMER_UPTIME).us);
    delay();
  }
}

int main(const char *args) {
  ioe_init();

  cmem_test();
  led_test();  
  seg_test();

  serial_test();
  timer_test();

  return 0;
}
