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
  return snprintf((char*)buf, len, 
    "WIDTH:%d\nHEIGHT: %d\n", 
    io_read(AM_GPU_CONFIG).width, 
    io_read(AM_GPU_CONFIG).height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
