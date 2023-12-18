#include <am.h>
#include <klib.h>
#include <switch.h>

static uint64_t boot_time = 0;

static inline uint64_t get_time() {
  uint32_t high, low;
  asm volatile (
    "lw %[high], 0(%[ahigh])\r\n"
    "lw %[low], 0(%[alow])\r\n"
    : [high] "=r"(high), [low] "=r"(low)
    : [alow] "r"(RTC_ADDR), [ahigh] "r"(RTC_ADDR + 4)
    : "memory", "t0", "t1"
  );
  return (uint64_t)low | (((uint64_t)high) << 32);
}

void __am_timer_init() {
  boot_time = get_time();
  for (int i = 0; i < 100; i ++) {
    for (volatile int j = 0; j < 1000; j ++);
    printf("[am-timer] Current: %lu\n", get_time());
  }
  halt(SWITCH_EXIT_SUCCESS);
  printf("[am-timer] Init, boot_time = %lu.\n", boot_time);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = get_time() - boot_time;
  printf("[am-timer] update: %lu.\n", uptime->us);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
