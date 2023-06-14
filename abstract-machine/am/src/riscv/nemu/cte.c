#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11:
      	switch(c->GPR1){
      		case -1:ev.event = EVENT_YIELD; break;
          case 4:ev.event = EVENT_IRQ_TIMER; break;
      		default:ev.event = EVENT_SYSCALL; break;
      	}
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

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
  Context *c = (Context *)((uint8_t *)(kstack.end) - sizeof(Context) - sizeof(uintptr_t));
  c->mepc = (uintptr_t)entry;
  c->mstatus = 0x1800;
  c->gpr[10] = (uintptr_t)arg;
  return c;
}

void yield() {
  asm volatile("li a7, -1; ecall");
  // asm volatile("li a7, 11; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
