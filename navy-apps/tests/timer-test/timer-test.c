#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <stdint.h>
#include <NDL.h>

int main(){
  /*
  NDL_Init(0);

  uint32_t time;
  uint32_t msec = 500;
  while (1) {
    time = NDL_GetTicks();
    while(time < msec) {
      time = NDL_GetTicks();
    }
    printf("get 0.5s !\n");
    msec += 500;
  }

  NDL_Quit();*/
  
  struct timeval dut;
  struct timezone tz;
  
  dut.tv_sec = 0;
  dut.tv_usec = 0;
  
  struct timeval ref;
  
  gettimeofday(&ref, &tz);
  ref.tv_usec = 500000;
  
  while(1){
    gettimeofday(&dut, &tz);
    
    if(dut.tv_sec == ref.tv_sec){
      if(ref.tv_usec == 0 && ref.tv_usec < dut.tv_usec){
        printf("now time: %ld.%lds\n", ref.tv_sec, ref.tv_usec / 1000);
        ref.tv_usec = 500 * 1000;
      }
      else if(ref.tv_usec == 500000 && ref.tv_usec < dut.tv_usec){
        printf("now time: %ld.%lds\n", ref.tv_sec, ref.tv_usec / 1000);
        ref.tv_usec = 0;
        ref.tv_sec += 1;
      }
    }
  }
}
