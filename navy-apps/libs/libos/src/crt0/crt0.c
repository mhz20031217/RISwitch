#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
extern void __libc_init_array (void);
void call_main(void *args) {
  __libc_init_array();
  int argc = *(int *) args;
  args += sizeof(int);
  char **argv = (char **)args;
  args += (argc + 1) * sizeof(char *);

  // set environ
  char **envp = (char **)args;
  char **eitem = envp;
  for (; *eitem != NULL; eitem ++) {
    char *name = strdup(*eitem);
    char *value;
    bool flag = false;
    for (value = name; *value != '\0'; value ++) {
      if (*value == '=') {
        *value = '\0';
        value ++;
        while (*value == ' ') {
          value ++;
        }
        break;
      }
    }

    if (flag) {
      setenv(name, value, 1);
    }
    
    free(name);
  }
  exit(main(argc, argv, envp));
  assert(0);
}
