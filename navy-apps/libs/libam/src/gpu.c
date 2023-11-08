#include "amdev.h"
#include <am.h>
#include <NDL.h>
#include <stdlib.h>
#include <string.h>

static int w, h;

static uint32_t *vmem;

void __am_gpu_init() {
  w = 400; h = 300;
  NDL_OpenCanvas(&w, &h);
  if (w == 0 || h == 0) {
    printf("[libam] Error opening canvas.\n");
    return;
  } else {
    vmem = malloc(w * h * 4);
    memset(vmem, 0, w * h * 4);
  }
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0x200000
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  for (int i = 0; i < ctl->h; i ++) {
    memcpy(vmem + (i+ctl->y)*w + ctl->x, (uint32_t *)ctl->pixels + i*ctl->w, ctl->w * 4);
  }
  if (ctl->sync) NDL_DrawRect(vmem, 0, 0, w, h);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
