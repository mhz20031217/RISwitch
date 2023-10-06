#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <stdbool.h>
#include <NDL.h>

// int main() {
//   struct timeval tv;
//   bool flag = 1;
//   while (1) {
//     gettimeofday(&tv, NULL);
//     // printf("gettimeofday: %ld, %ld\n", tv.tv_sec, tv.tv_usec);
//     if (flag) {
//       if (tv.tv_usec < 500000) {
//         printf("%ld %ld\n", tv.tv_sec, tv.tv_usec);
//         flag = 0;
//       }
//     } else {
//       if (tv.tv_usec < 500000) {
//         continue;
//       }
//       printf("%ld %ld\n", tv.tv_sec, tv.tv_usec);
//       flag = 1;
//     }
//   }

//   return 0;
// }

int main() {
  bool flag = 1;
  while (1) {
    uint32_t cur = NDL_GetTicks();
    uint32_t msec = cur % 1000;
    printf("cur: %u\n", cur);
    if (flag) {
      if (msec < 500) {
        printf("%u\n", cur);
        flag = 0;
      }
    } else {
      if (msec < 500) {
        continue;
      }
      printf("%u\n", cur);
      flag = 1;
    }
  }

  return 0;
}
