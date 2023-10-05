#include <common.h>
#include <sys/types.h>
#include "syscall.h"

int SYS_exit(uintptr_t args[]) {
  halt(args[0]);
  return 0;
}

int SYS_yield(uintptr_t args[]) {
  yield();
  return 0;
}

static struct {
  int (*handler)(uintptr_t args[]);
  const char *desc;
} syscall_handler[] = {
  {SYS_exit, "exit"},
  {SYS_yield, "yield"}
};

#define NR_SYSCALL ARRLEN(syscall_handler)

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  if (a[0] >= NR_SYSCALL) {
    if (a[0] == (uintptr_t)-1) {
      c->GPRx = syscall_handler[1].handler(a);
      return;
    }
    panic("Unhandled syscall ID = %d", a[0]);
  } else {
    c->GPRx = syscall_handler[a[0]].handler(a);
  }
}
