#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
	// int i;
	// int temp = inl(VGACTL_ADDR);
	// int w = temp >> 16;
	// int h = temp & 0xFFFF;
	// uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	// for (i = 0; i < w * h; i++) fb[i] = i;
	// outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
	int temp = inl(VGACTL_ADDR);
	int width = temp >> 16;
	int height = temp & 0xFFFF;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
	int i, j;
	int screen_width = inl(VGACTL_ADDR) >> 16;
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	uint32_t *pixels = ctl->pixels;
	for (i = ctl->y; i < ctl->y + ctl->h; i++)
		for (j = ctl->x; j < ctl->x + ctl->w; j++) {
			fb[screen_width*i+j] = pixels[ctl->w*(i-ctl->y)+(j-ctl->x)];
		}

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }

}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}