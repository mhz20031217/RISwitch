#include <stdio.h>

extern "C" void serial_write(char ch) {
  putchar(ch);
}
