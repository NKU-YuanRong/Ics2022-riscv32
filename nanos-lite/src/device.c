#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(int i = 0;i < len;i++){
    putch(((char*)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  size_t actual_len = 1;
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  actual_len = sprintf(buf, "%s %s\n", ev.keydown ? "kd": "ku", keyname[ev.keycode]);
  return actual_len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int actual_len = snprintf((char*)buf, len, 
    "screen width: %d, height: %d\n", 
    io_read(AM_GPU_CONFIG).width, 
    io_read(AM_GPU_CONFIG).height);
  // Log("display infomation: %s", (char*)buf);
  return actual_len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int x, y, w, h, actual_len;
  w = io_read(AM_GPU_CONFIG).width;
  h = io_read(AM_GPU_CONFIG).height;
  x = (offset / 4) % w;
  y = (offset / 4) / w;
  actual_len = offset + len > w * h * 4 ? w * h * 4 - offset : len;

  io_write(AM_GPU_FBDRAW, x, y, (uint32_t*)buf, actual_len / 4, 1, true);
  return actual_len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
