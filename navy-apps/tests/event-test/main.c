#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  while (1) {
    char buf[64];
    uint32_t ret = NDL_PollEvent(buf, sizeof(buf));
    printf("ret value: %d\n", ret);
    if (ret != 0) {
      printf("receive event: %s\n", buf);
    }
  }
  return 0;
}
