#include <am.h>
#include <klib-macros.h>
#include <switch.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
  outl(SERIAL_PORT, (int)ch);
}

/* Terminate AM Program execution by calling this function
 * 
 * @param code should be SWITCH_EXIT_SUCCESS or SWITCH_EXIT_FAIL
 *
 * This function does not return.
 * */
void halt(int code) {
  switch_trap(code);

  // should always reach here
  // The cpu will determine this condition and in NVDL C++ wrapper,
  // it will terminate simulation automatically.
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
