#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <proc.h>
#include <sys/time.h>

/*
enum {
  SYS_exit,
  SYS_yield,
  SYS_open,
  SYS_read,
  SYS_write,
  SYS_kill,
  SYS_getpid,
  SYS_close,
  SYS_lseek,
  SYS_brk,
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};*/

extern void naive_uload(void *pcb, const char *filename);

int sys_gettimeofday(Context *c){
  struct timeval *tv = (struct timeval *)c->GPR2;
  __uint64_t time = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = (time % 1000000);
  tv->tv_sec = (time / 1000000);
	return 0;
}

int sys_write(Context *c) {
  if (c->GPR2 == 1 || c->GPR2 == 2){
    for (int i = 0; i < c->GPR4; ++i){
      putch(*(((char *)c->GPR3) + i));
    }
    return c->GPR4;
  }
  else  
    return fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_yield: yield(); break;
    case SYS_exit: halt(0); break;
    case SYS_brk: c->GPRx = 0; break;
    case SYS_open: c->GPRx = fs_open((const char*)(a[1])); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_read: c->GPRx = fs_read(a[1], (void*)(a[2]), a[3]); break;
    case SYS_write: c->GPRx = sys_write(c); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_gettimeofday: c->GPRx = sys_gettimeofday((void *)a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
