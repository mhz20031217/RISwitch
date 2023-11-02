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

static void sh_handle_cmd(const char *cmd) {
  char exec[256], arg[256];
  sscanf(cmd, "%s", exec);
  sscanf(cmd + strlen(exec) + 1, "%[^\n]", arg);
  if (strcmp(exec, "exit") == 0) {
    exit(0);
  } else if (strcmp(exec, "echo") == 0) {
    sh_printf("%s\n", arg);
  } else {
    char abs_path[256];
    FILE *fp;
    const char *PATH = getenv("PATH");
    printf("[nterm] Current PATH: '%s'.\n", PATH);
    int len = strlen(PATH), i = 0;
    for (int j = 0; i < len; i ++) {
      if (PATH[i] == ':') {
        int p;
        for (p = 0; p < j-i; p ++) {
          abs_path[p] = PATH[i+p];
        }
        abs_path[p] = '/'; abs_path[p+1] = '\0';
        strcat(abs_path, exec);
        fp = fopen(exec, "r");
        if (fp) {
          break;
        }
      }
    }
    if (!fp) {
      sh_printf("[sh] no such file or directory: '%s'.\n", exec);
    } else {
      fclose(fp);
      execve(abs_path, NULL, NULL);
    }
  }
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
