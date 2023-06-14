#include <common.h>
#include <proc.h>

extern void do_syscall(Context*);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
  	case 1: // Log("Yield handle successfully!"); c->mepc += 4; break;
      Log("new yield!"); c->GPRx = (unsigned int)schedule(c); c->mepc += 4; break;
    case 2: do_syscall(c); c->mepc += 4; break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
