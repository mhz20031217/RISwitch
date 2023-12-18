#include <stdio.h>

extern "C" void serial_write(int ch) {
  putchar(ch);
}
