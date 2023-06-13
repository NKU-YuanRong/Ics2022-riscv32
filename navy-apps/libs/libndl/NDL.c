#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int fbctr, fb;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);  
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fp = open("/dev/events");
  return read(fp, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  else{
    if(*w == 0 && *h == 0){
      char buf[64];
      read(fbctr, buf, 64);
      sscanf(buf, "WIDTH:%d\nHEIGHT:%d\n", &canvas_w, &canvas_h);
      *w = canvas_w;
      *h = canvas_h;
    }
    else{
      canvas_w = *w;
      canvas_h = *h;
    }
    printf("w=%d\th=%d\n",canvas_w,canvas_h);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for (int i = 0; i < h; i++) {
    lseek(fb, ((y + i) * screen_w + x) * 4, SEEK_SET); 
    write(fb, (pixels + i * w), w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  char buf[64];
  fbctr=open("/proc/dispinfo",0,0);
  fb=open("/dev/fb",2,0);
  printf("%d\n",fbctr);
  read(fbctr,buf,64);
  sscanf(buf,"WIDTH:%d\nHEIGHT:%d\n",&screen_w,&screen_h);
  return 0;
}

void NDL_Quit() {
}
