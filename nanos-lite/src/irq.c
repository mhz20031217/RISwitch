#include <common.h>
#include <proc.h>

Context *do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      c = schedule(c);
      break;
    case EVENT_SYSCALL:
      c = do_syscall(c);
      break;
    case EVENT_IRQ_TIMER:
      Log("Timer event.");
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
