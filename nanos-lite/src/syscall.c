#include <common.h>
#include <sys/types.h>
#include <sys/time.h>
#include "am.h"
#include "syscall.h"
#include <fcntl.h>
#include <debug.h>
#include <fs.h>
#include <loader.h>
#include <memory.h>

static inline int sys_exit(intptr_t args[]) {
  context_uload(
    current,
    "/bin/menu", 
    NULL, 
    NULL
  );
  args[0] = (intptr_t) current->cp;
  return current->cp->GPRx;
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
  int fd = fs_open((const char *)args[1], O_SYNC, O_CREAT);
  if (fd == -1) {
    return -1;
  }
  fs_close(fd);
  context_uload(
    current,
    (const char *)args[1], 
    (char *const *)args[2], 
    (char *const *)args[3]
  );
  args[0] = (intptr_t) current->cp;
  printf("sys_execve returned.\nNew stack top at: %p.\n", current->cp->GPRx);
  return -1;
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

Context* do_syscall(Context *c) {
  int syscallid = c->GPR1;
  intptr_t args[4];
  args[0] = (intptr_t) c;
  args[1] = c->GPR2;
  args[2] = c->GPR3;
  args[3] = c->GPR4;

  if (syscallid >= NR_SYSCALL) {
    panic("Unhandled syscall ID = %d", syscallid);
  } else {
    #ifdef ENABLE_STRACE
    printf("[strace] %s(%lx, %lx, %lx) ", syscall_handler[syscallid].desc, args[1], args[2], args[3]);
    #endif
    if (syscall_handler[syscallid].handler == NULL) {
      panic("Unhandled syscall ID = %d", args[0]);
    }
    c->GPRx = syscall_handler[syscallid].handler(args);
    #ifdef ENABLE_STRACE
    switch (syscallid) {
      case SYS_open:
        printf("on file '%s' = %d\n", (const char *) args[1], c->GPRx); break;
      case SYS_lseek: case SYS_close: case SYS_read: case SYS_write:
        printf("on file '%s' = %d\n", fs_getfilename(args[1]), c->GPRx); break;
      default: printf("= %u (%d, 0x%x)\n", c->GPRx, c->GPRx, c->GPRx); break;
    }
    #endif
  }
  return (Context *)args[0];
}
