#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (!user_handler) return c;
  Event ev = {0};
  switch (c->mcause) {
    case 11:
      if (c->GPR1 < 20) ev.event = EVENT_SYSCALL;
      else ev.event = EVENT_YIELD;
      c->mepc += 4;
      break;
    default: ev.event = EVENT_ERROR; break;
  }
  c = user_handler(ev, c);
  assert(c != NULL);

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *kthread = (Context *) ((unsigned char *) kstack.end - sizeof(Context));
  kthread->mepc = (uintptr_t) entry;
  kthread->mstatus = 0x1800;
  kthread->mcause = 11; // TODO: check correctness
  *(uintptr_t *) kstack.start = (uintptr_t) kthread;
  return kthread;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
