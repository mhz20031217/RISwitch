#include "amdev.h"
#include "riscv/riscv.h"
#include <am.h>
#include <nemu.h>
#include <stdint.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_LOCK_ADDR      (AUDIO_ADDR + 0x18)

static int bufsize;

void __am_audio_init() {
  bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  int lock;
  while ((lock = inl(AUDIO_LOCK_ADDR)) != 0);
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  char *start = ctl->buf.start, *end = ctl->buf.end;
  int len = end - start, lock, count;
  while ((lock = inl(AUDIO_LOCK_ADDR)) != 0
    || bufsize - (count = inl(AUDIO_COUNT_ADDR)) < len);
  outl(AUDIO_LOCK_ADDR, 1);

  uintptr_t p = AUDIO_SBUF_ADDR + count;
  while (start < end) {
    outl(p, *start);
    start ++;
    p ++;
  }
  outl(AUDIO_COUNT_ADDR, count + len);
  outl(AUDIO_LOCK_ADDR, 0);
}
