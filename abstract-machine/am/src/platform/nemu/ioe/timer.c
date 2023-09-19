#include <am.h>
#include <amdev.h>
#include <nemu.h>

static uint64_t boot_time = 0;

void __am_timer_init() {
  boot_time = 0;
  boot_time |= inl(AM_TIMER_UPTIME);
  boot_time |= (uint64_t) inl(AM_TIMER_UPTIME + 4) << 32;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t cur_time = 0;
  cur_time |= inl(AM_TIMER_UPTIME);
  cur_time |= (uint64_t) inl(AM_TIMER_UPTIME + 4) << 32;
  uptime->us = cur_time;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
