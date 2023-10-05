#include <common.h>
#include <sys/types.h>
#include "syscall.h"

static inline int sys_exit(uintptr_t args[]) {
  halt(args[0]);
  return 0;
}

static inline int sys_yield(uintptr_t args[]) {
  yield();
  return 0;
}

static inline int sys_write(uintptr_t args[]) {
  int fd = args[1];
  const void *buf = (const void *) args[2];
  size_t count = args[3];
  // printf("Called SYS_write, fd = %d, buf = 0x%p, count = %lu.\n", fd, buf, count);

  if (fd == 1 || fd == 2) {
    for (size_t i = 0; i < count; i ++) {
      putch(*((const char *)(buf) + i));
    }
    return count;
  } else {
    return -1;
  }
}

static inline int sys_brk(uintptr_t args[]) {
  // void *addr = args[1];
  return 0;
}

static struct {
  int (*handler)(uintptr_t args[]);
  const char *desc;
} syscall_handler[] = {
  [SYS_exit] = {sys_exit, "exit"},
  [SYS_yield] = {sys_yield, "yield"},
  [SYS_write] = {sys_write, "write"},
  [SYS_brk] = {sys_brk, "brk"},
};

#define NR_SYSCALL ARRLEN(syscall_handler)

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  if (a[0] >= NR_SYSCALL) {
    panic("Unhandled syscall ID = %d", a[0]);
  } else {
    #ifdef ENABLE_STRACE
    printf("[strace] %s(%lx, %lx, %lx) = ", syscall_handler[a[0]].desc, a[1], a[2], a[3]);
    #endif
    c->GPRx = syscall_handler[a[0]].handler(a);
    #ifdef ENABLE_STRACE
    printf("%u (%d, 0x%x)\n", c->GPRx);
    #endif
  }
}
