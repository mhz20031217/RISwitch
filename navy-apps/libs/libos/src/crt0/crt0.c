#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
extern void __libc_init_array (void);
void call_main(void *args) {
  char *empty[] =  {NULL };
  environ = empty;
  __libc_init_array();
  int argc = *(int *) args;
  args += sizeof(int);
  char **argv = *(char ***)args;
  args += (argc + 1) * sizeof(char *);
  char **envp = *(char ***)args;
  exit(main(argc, argv, envp));
  assert(0);
}
