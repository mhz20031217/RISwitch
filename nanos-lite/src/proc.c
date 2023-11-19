#include "am.h"
#include <proc.h>
#include <loader.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    Log("The message is '%s'.", arg);
    j ++;
    yield();
  }
}

void my_kthread(void *arg) {
  static int i = 100000;
  while (1) {
    if (i == 100000) {
      Log("kthread is running: %s.", arg);
      i = 0;
    } else {
      i ++;
    }
    yield();
  }
}

void init_proc() {
  Log("Initializing kthreads...");
  context_kload(&pcb[0], my_kthread, "A is running.");
  // context_kload(&pcb[1], hello_fun, "B is running.");

  Log("Initializing processes...");
  switch_boot_pcb();
  context_uload(&pcb[2], "/bin/menu", NULL, NULL);

  Log("Loaded.");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  if (current < pcb || current >= pcb + MAX_NR_PROC) {
    current = &pcb[0];
  } else {
    while (current->cp == prev || current->cp == NULL) {
      if (current == pcb + MAX_NR_PROC - 1) {
        current = pcb;
      } else {
        current += 1;
      }
    }
  }
  return current->cp;
}
