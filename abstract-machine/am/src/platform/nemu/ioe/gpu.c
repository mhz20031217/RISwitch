#include "amdev.h"
#include "riscv/riscv.h"
#include <am.h>
#include <nemu.h>
#include <stdint.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int w, h;

void __am_gpu_init() {
  // int i;
  w = inl(VGACTL_ADDR) >> 16;
  h = inl(VGACTL_ADDR) & 0xffff;
  // printf("AM GPU initialzed, width: %d, height: %d.\n", w, h);
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < w * h; i ++) fb[i] = 0;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0x200000
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  // printf("drawing a %d, %d rect at (%d, %d)\n", ctl->w, ctl->h, ctl->x, ctl->y);
  for (int j = 0; j < ctl->h; j ++) {
    for (int i = 0; i < ctl->w; i ++) {
      outl(
        FB_ADDR + 4 * ((ctl->y + j) * w + (ctl->x + i)),
        *((uint32_t *) ctl->pixels + ctl->w * j + i)
      );
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
