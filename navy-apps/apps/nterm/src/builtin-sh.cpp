#include "sys/unistd.h"
#include <nterm.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

// static void sh_handle_cmd(const char *cmd) {
//   char exec[256], arg[256];
//   sscanf(cmd, "%s", exec);
//   sscanf(cmd + strlen(exec) + 1, "%[^\n]", arg);
//   if (strcmp(exec, "exit") == 0) {
//     exit(0);
//   } else if (strcmp(exec, "echo") == 0) {
//     sh_printf("%s\n", arg);
//   } else {
//     char abs_path[256];
//     char PATH[256];
//     FILE *fp;
//     strcpy(PATH, getenv("PATH"));
//     printf("[nterm] Current PATH: '%s'.\n", PATH);
//     strcat(PATH, ":");
//     int len = strlen(PATH), i = 0, j;
//     for (j = 0; j < len; j ++) {
//       if (PATH[j] == ':') {
//         int p;
//         for (p = 0; p < j-i; p ++) {
//           abs_path[p] = PATH[i+p];
//         }
//         abs_path[p] = '/'; abs_path[p+1] = '\0';
//         strcat(abs_path, exec);
//         printf("[nterm] Trying to exec '%s'.\n", abs_path);
//         fp = fopen(abs_path, "r");
//         if (fp) {
//           break;
//         }
//         i = j + 1;
//         j ++;
//       }
//     }
//     if (!fp) {
//       sh_printf("[sh] no such file or directory: '%s'.\n", exec);
//     } else {
//       fclose(fp);
//       execve(abs_path, NULL, NULL);
//     }
//   }
// }

static void sh_handle_cmd(const char *cmd) {
  char *command = strdup(cmd);

  const char *delim = " \n\t";
  int argc = 0;
  char **argv = NULL, **envp = NULL;

  char *token = strtok(command, delim);
  while (token != NULL) {
    argc ++;
    token = strtok(NULL, delim);
  }

  if (argc > 0) {
    argv = (char **) malloc(sizeof(const char *) * (argc + 1));
    command = strdup(cmd);
    
    argc = 0;
    token = strtok(command, delim);
    while (token != NULL) {
      argv[argc ++] = token;
      token = strtok(NULL, delim);
    }
  }

  if (execvp(argv[0], argv) == -1) {
    sh_printf("[sh] No such file or directory: '%s'.\n", argv[0]);
  }

  free(argv);
  free(command);
}

void builtin_sh_run() {
  setenv("PATH", "/bin", 1);
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
