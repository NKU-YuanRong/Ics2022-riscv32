#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <stdint.h>
#include <NDL.h>

int main(){
  struct timeval dut;
  struct timezone tz;
  struct timeval ref;
  
  dut.tv_sec = 0;
  dut.tv_usec = 0;
  
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
  
  return 0;
}
