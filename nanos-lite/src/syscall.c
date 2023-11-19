#include <common.h>
#include <sys/types.h>
#include <sys/time.h>
#include "am.h"
#include "syscall.h"
#include <debug.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>

static inline int sys_exit(intptr_t args[]) {
  naive_uload(NULL, "/bin/menu");
  return 0;
}

static inline int sys_yield(intptr_t args[]) {
  yield();
  return 0;
}

static inline int sys_write(intptr_t args[]) {
  int fd = args[1];
  const void *buf = (const void *) args[2];
  size_t count = args[3];
  return fs_write(fd, buf, count);
}

static inline int sys_brk(intptr_t args[]) {
  return mm_brk(args[1]);
}

static inline int sys_open(intptr_t args[]) {
  return fs_open((const char *) args[1], args[2], args[3]);
}

static inline int sys_close(intptr_t args[]) {
  return fs_close(args[1]);
}

static inline int sys_lseek(intptr_t args[]) {
  return fs_lseek(args[1], args[2], args[3]);
}

static inline int sys_read(intptr_t args[]) {
  return fs_read(args[1], (void *) args[2], args[3]);
}

static inline int sys_gettimeofday(intptr_t args[]) {
  struct timeval *tv = (void *) args[1];
  if (tv == NULL) return 0;
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = us % 1000000;
  tv->tv_sec = us / 1000000;

  return 0;
}

static inline int sys_execve(intptr_t args[]) {
  // naive_uload(NULL, (char *) args[1]);
  context_uload(
    current,
    (const char *)args[1], 
    (char *const *)args[2], 
    (char *const *)args[3]
  );
  printf("sys_execve returned.\nNew stack top at: %p.\n", current->cp->GPRx);
  return current->cp->GPRx;
}

static struct {
  int (*handler)(intptr_t args[]);
  const char *desc;
} syscall_handler[] = {
  [SYS_exit] = {sys_exit, "exit"},
  [SYS_yield] = {sys_yield, "yield"},
  [SYS_write] = {sys_write, "write"},
  [SYS_brk] = {sys_brk, "brk"},
  [SYS_read] = {sys_read, "read"},
  [SYS_lseek] = {sys_lseek, "lseek"},
  [SYS_open] = {sys_open, "open"},
  [SYS_close] = {sys_close, "close"},
  [SYS_gettimeofday] = {sys_gettimeofday, "gettimeofday"},
  [SYS_execve] = {sys_execve, "execve"},
};

#define NR_SYSCALL ARRLEN(syscall_handler)

void do_syscall(Context *c) {
  intptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  if (a[0] >= NR_SYSCALL) {
    panic("Unhandled syscall ID = %d", a[0]);
  } else {
    #ifdef ENABLE_STRACE
    printf("[strace] %s(%lx, %lx, %lx) ", syscall_handler[a[0]].desc, a[1], a[2], a[3]);
    #endif
    if (syscall_handler[a[0]].handler == NULL) {
      panic("Unhandled syscall ID = %d", a[0]);
    }
    c->GPRx = syscall_handler[a[0]].handler(a);
    #ifdef ENABLE_STRACE
    switch (a[0]) {
      case SYS_open:
        printf("on file '%s' = %d\n", (const char *) a[1], c->GPRx); break;
      case SYS_lseek: case SYS_close: case SYS_read: case SYS_write:
        printf("on file '%s' = %d\n", fs_getfilename(a[1]), c->GPRx); break;
      default: printf("= %u (%d, 0x%x)\n", c->GPRx, c->GPRx, c->GPRx); break;
    }
    
    #endif
  }
}
