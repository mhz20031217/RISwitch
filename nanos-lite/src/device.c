#include "amdev.h"
#include <common.h>
#include <stdbool.h>
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
  yield();
  const char *src = buf;

  size_t cnt = 0;
  while (cnt < len) {
    putch(*(src ++));
    cnt ++;
  }

  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  yield();
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  int rc = snprintf(buf, len, "k%c %s\n", ev.keydown ? 'd' : 'u', keyname[ev.keycode]);
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
  yield();
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

static AM_AUDIO_CONFIG_T audio_config;

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
  audio_config = io_read(AM_AUDIO_CONFIG);
  if (audio_config.present == false) {
    return -1;
  }

  const int *args = buf;
  int freq = args[0], channels = args[1], samples = args[2];
  Log("Writing sbctl: freq = %d, channels = %d, samples = %d.", freq, channels, samples);

  io_write(AM_AUDIO_CTRL, freq, channels, samples);

  return 12;
}

size_t sbctl_read(void *buf, size_t offset, size_t len) {
  if (audio_config.present == false) {
    return -1;
  }

  *(int *) buf = audio_config.bufsize - io_read(AM_AUDIO_STATUS).count;

  return 4;
}

size_t sb_write(const void *buf, size_t offset, size_t len) {
  if (audio_config.present == false) {
    return -1;
  }

  io_write(AM_AUDIO_PLAY, { .start = (char *) buf, .end = (char *) buf + len });
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
