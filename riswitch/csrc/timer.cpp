#include <common.hpp>

static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec/1000;
  return us;
}

uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

extern "C" void timer_read(int is_high, word_t *data) {
  // printf("timer_read(%s)\n", (is_high) ? "high" : "low");
  static uint64_t now = 0;
  if (is_high) {
    *data = (now >> 32);
  } else {
    now = get_time();
    *data = now;
  }
  // printf("[cpp timer] return %u\n", (uint32_t) *data);
}
