#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      printf("To ensure running on the user stack, address of j is %p.\n", &j);
      j = 0;
    }
  }
  return 0;
}
