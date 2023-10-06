#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <stdbool.h>

int main() {
  struct timeval tv;
  bool flag = 1;
  while (1) {
    gettimeofday(&tv, NULL);
    if (flag) {
      if (tv.tv_usec < 5000000) {
        printf("%ld %ld", tv.tv_sec, tv.tv_usec);
        flag = 0;
      }
    } else {
      if (tv.tv_usec < 5000000) {
        continue;
      }
      printf("%ld %ld", tv.tv_sec, tv.tv_usec);
      flag = 1;
    }
  }

  return 0;
}
