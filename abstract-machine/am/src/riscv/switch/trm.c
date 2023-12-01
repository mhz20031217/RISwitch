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
  outl(SEG_ADDR, (unsigned char)ch);
}

/* Terminate AM Program execution by calling this function
 * 
 * @param code should be SWITCH_EXIT_SUCCESS or SWITCH_EXIT_FAIL
 *
 * This function does not return.
 * */
void halt(int code) {
  switch_trap(code);

  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
