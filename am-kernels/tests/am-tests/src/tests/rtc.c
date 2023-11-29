#include <amtest.h>

// void rtc_test() {
//   AM_TIMER_RTC_T rtc;
//   int sec = 1;
//   while (1) {
//     while(io_read(AM_TIMER_UPTIME).us / 1000000 < sec) ;
//     rtc = io_read(AM_TIMER_RTC);
//     printf("%d-%d-%d %02d:%02d:%02d GMT (", rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
//     if (sec == 1) {
//       printf("%d second).\n", sec);
//     } else {
//       printf("%d seconds).\n", sec);
//     }
//     sec ++;
//   }
// }

void rtc_test() {
  AM_TIMER_RTC_T rtc;
  int msec = 1;
  while (1) {
    while(io_read(AM_TIMER_UPTIME).us / 1000 < msec) ;
    rtc = io_read(AM_TIMER_RTC);
    printf("%d-%d-%d %02d:%02d:%02d GMT (", rtc.year, rtc.month, rtc.day, rtc.hour, rtc.minute, rtc.second);
    if (msec == 1) {
      printf("%d milisecond).\n", msec);
    } else {
      printf("%d miliseconds).\n", msec);
    }
    msec ++;
  }
}
