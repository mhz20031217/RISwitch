#include <common.h>
#include <stddef.h>
#include <stdio.h>

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
  const char *src = buf;

  size_t cnt = 0;
  while (cnt < len) {
    putch(*(src ++));
    cnt ++;
  }

  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  int rc = snprintf(buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  return rc;
}

static AM_GPU_CONFIG_T gpu_config;

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  gpu_config = io_read(AM_GPU_CONFIG);
  if (!gpu_config.present) {
    return -1;
  }
  Log("Reading dispinfo: %dx%d.", gpu_config.width, gpu_config.height);
  int rc = snprintf(buf, len, "WIDTH : %d\nHEIGHT : %d\n",
            gpu_config.width, gpu_config.height);
  if (rc <= 0) {
    return -1;
  } else {
    return rc;
  }
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  offset >>= 2;
  len >>= 2;
  io_write(AM_GPU_FBDRAW, 
    .pixels = (void *) buf,
    .w = len, .h = 1,
    .x = offset % gpu_config.width,
    .y = offset / gpu_config.width,
    .sync = true
  );
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
