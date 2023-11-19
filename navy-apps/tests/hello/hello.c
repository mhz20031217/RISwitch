#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
  write(1, "Hello World!\n", 13);

  printf("argc: %d, argv: %p, envp: %p\n", argc, argv, envp);
  if (argv) {
    int i = 0;
    while (*argv != NULL) {
      printf("argv[%d]: %s", i, *argv);
      argv ++;
      i ++;
    }
  }
  if (envp) {
    int i = 0;
    while (*envp != NULL) {
      printf("envp[%d]: %s", i, *argv);
      argv ++;
      i ++;
    }
  }

  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      printf("To ensure running on the user stack, address of j is %p.\n", &j);
      j = 0;
    }
  }
  return 0;
}
