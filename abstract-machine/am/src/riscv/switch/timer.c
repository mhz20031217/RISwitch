#include <am.h>
#include <klib.h>
#include <switch.h>

static uint64_t boot_time = 0;

static inline uint64_t get_time() {
  uint64_t now = inl(RTC_ADDR);
  return (now | ((uint64_t)inl(RTC_ADDR + 4)) << 32);
}

void __am_timer_init() {
  boot_time = get_time();
  printf("[am-timer] Init, boot_time = %ld.\n", boot_time);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = get_time() - boot_time;
  printf("[am-timer] update: %ld.\n", uptime->us);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
