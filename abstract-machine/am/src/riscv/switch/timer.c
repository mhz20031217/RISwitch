#include <am.h>
#include <klib.h>
#include <switch.h>

static uint64_t boot_time = 0;

static inline uint64_t get_time() {
  volatile uint64_t now;
  volatile uint64_t low, high;
  low = inl(RTC_ADDR);
  high = inl(RTC_ADDR + 4);
  now = (high << 32) | low;
  return now;
}

void __am_timer_init() {
  boot_time = get_time();
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = get_time() - boot_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
