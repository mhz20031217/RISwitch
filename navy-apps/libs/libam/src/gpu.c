#include "amdev.h"
#include <am.h>
#include <NDL.h>

static int w, h;

void __am_gpu_init() {
  w = 400; h = 300;
  NDL_OpenCanvas(&w, &h);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0x200000
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  NDL_DrawRect(ctl->pixels, ctl->x, ctl->y, ctl->w, ctl->y);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
