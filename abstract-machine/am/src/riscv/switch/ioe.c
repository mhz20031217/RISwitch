#include <am.h>
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
  led->value = inl(LED_ADDR);
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
  [AM_CMEM_PUTCH  ] = __am_cmem_putch
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
