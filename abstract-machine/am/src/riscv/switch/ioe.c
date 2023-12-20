#include "amdev.h"
#include <am.h>
#include <stdbool.h>
#include <switch.h>
#include <klib-macros.h>

void __am_timer_init();

void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_cmem_config(AM_CMEM_CONFIG_T *cfg) { cfg->present = true; cfg->width = 70; cfg->height = 30; }

static void __am_seg(AM_SEG_T* seg) {
  outl(SEG_ADDR, seg->value);
}

static void __am_led(AM_LED_T* led) {
  outl(LED_ADDR, led->value);
}

static void __am_switch(AM_SWITCH_T* s) {
  s->value = inl(SW_ADDR);
}

static void __am_cmem_putch(AM_CMEM_PUTCH_T* ch) {
  uintptr_t addr = CMEM_ADDR + (ch->x*32+ch->y)*2;
  uint16_t cell = 
    ((unsigned char)ch->ascii) | (((uint16_t)ch->fg & 0x7U) << 8) | (((uint16_t)ch->bg & 0x7U) << 11);
  outw(addr, cell);
}

static void __am_serial_putch(AM_SERIAL_PUTCH_T *ch) {
  outl(SERIAL_PORT, (int)ch->ch);
}

#define VGA_W 640
#define VGA_H 480

static void __am_vgactrl(AM_VGACTRL_T *c) {
  if (c->mode) {
    outl(VGACTL_ADDR, 1);
  } else {
    outl(VGACTL_ADDR, 0);
  }
}

static void __am_gpu_config(AM_GPU_CONFIG_T *c) {
  c->present = true;
  c->has_accel = false;
  c->width = VGA_W;
  c->height = VGA_H;
  c->vmemsz = VGA_W * VGA_H * 4;
  // Though we only support 16-bit color in RISwitch,
  // AM Programs might assert 32-bit color
}

static void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *c) {
  if (!c) return;
  if (!c->pixels) return;
  int x = c->x, y = c->y, w = c->w, h = c->h;
  if (x < 0 || y < 0 || x + w > VGA_W || y + h > VGA_H) {
    return;
  }
  
  uint32_t *pixels = c->pixels;
  uint16_t *buf = (uint16_t *)FB_ADDR;
  for (int i = 0; i < c->w; i ++) {
    for (int j = 0; j < c->h; j ++) {
      uintptr_t addr = (uintptr_t)(buf + ((x+i) * 512 + (y+j)));
      uint32_t pixel = *(pixels + (w*j + i));
      uint16_t data = 
        ((pixel >> 20) << 8) | 
        ((pixel & 0x0f000) >> 8) |
        ((pixel & 0x0f0) >> 4);
      outl(addr, data);
    }
  }
}

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_SEG         ] = __am_seg,
  [AM_SWITCH      ] = __am_switch,
  [AM_LED         ] = __am_led,
  [AM_CMEM_CONFIG ] = __am_cmem_config,
  [AM_CMEM_PUTCH  ] = __am_cmem_putch,
  [AM_SERIAL_PUTCH] = __am_serial_putch,
  [AM_VGACTRL     ] = __am_vgactrl,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw
};

static void fail(void *buf) { panic("access nonexist register"); }

bool ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  __am_timer_init();
  return true;
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
