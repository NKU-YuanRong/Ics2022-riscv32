#include <proc.h>

#define MAX_NR_PROC 4

extern void naive_uload(PCB*, const char*);

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

// void context_uload(PCB* pcb,const char *filename);

void context_kload(PCB* create_pcb,void (*entry)(void*),void *arg){
  Area stack = {create_pcb->stack, create_pcb->stack + STACK_SIZE};
  create_pcb->cp = kcontext(stack, entry, arg);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (char*)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  // char a[5] = "yyy";
  context_kload(&pcb[0], hello_fun, NULL);
  context_kload(&pcb[1], hello_fun, NULL);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/dummy");
  // naive_uload(NULL, "/bin/hello");
  // naive_uload(NULL, "/bin/file-test");
  // naive_uload(NULL, "/bin/timer-test");
  // naive_uload(NULL, "/bin/event-test");
  // naive_uload(NULL, "/bin/bmp-test");
  // naive_uload(NULL, "/bin/menu");
  // naive_uload(NULL, "/bin/nterm");
  // naive_uload(NULL, "/bin/bird");
  // naive_uload(NULL, "/bin/pal");
}

bool ret = false;

Context* schedule(Context *prev) {
  current->cp = prev;
  if (ret) {
    Log("Set pcb1!");
    current = &pcb[1];
  }
  else {
    Log("Set pcb0!");
    current = &pcb[0];
  }
  ret = !ret;
  return current->cp;
}
