#include <common.h>
#include <sys/types.h>
#include "syscall.h"

static inline int SYS_exit(uintptr_t args[]) {
  halt(args[0]);
  return 0;
}

static inline int SYS_yield(uintptr_t args[]) {
  yield();
  return 0;
}

static inline int SYS_write(uintptr_t args[]) {
  int fd = args[1];
  const void *buf = (const void *) args[2];
  size_t count = args[3];

  if (fd == 1 || fd == 2) {
    for (size_t i = 0; i < count; i ++) {
      putch(*((const char *)(buf) + i));
    }
    return count;
  } else {
    return -1;
  }
}

// enum {
//   SYS_exit,    0
//   SYS_yield,   1
//   SYS_open,    2
//   SYS_read,    3
//   SYS_write,   4
//   SYS_kill,    5
//   SYS_getpid,  6
//   SYS_close,   7
//   SYS_lseek,   8
//   SYS_brk,     9
//   SYS_fstat,   10
//   SYS_time,    11
//   SYS_signal,  12
//   SYS_execve,  13
//   SYS_fork,    14
//   SYS_link,    15
//   SYS_unlink,  16
//   SYS_wait,    17
//   SYS_times,   18
//   SYS_gettimeofday 19
// };

static struct {
  int (*handler)(uintptr_t args[]);
  const char *desc;
} syscall_handler[] = {
  {SYS_exit, "exit"},     // 0
  {SYS_yield, "yield"},
  {NULL, NULL},
  {NULL, NULL},
  {SYS_write, "write"},   // 4
};

#define NR_SYSCALL ARRLEN(syscall_handler)

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  if (a[0] >= NR_SYSCALL) {
    panic("Unhandled syscall ID = %d", a[0]);
  } else {
    #ifdef ENABLE_STRACE
    printf("[strace] %s\n", syscall_handler[a[0]].desc);
    #endif
    c->GPRx = syscall_handler[a[0]].handler(a);
  }
}
