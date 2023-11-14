/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <errno.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <cpu/difftest.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include "debug.h"
#include "sdb.h"
#include "utils.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args) {
  bool success;
  word_t result = expr(args, &success);
  if (!success) {
    Info("Invalid EXPR: %s", args);
  } else {
    printf("%u 0x%x\n", result, result);
  }
  return 0;
}

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_attach(char *args) {
  difftest_toggle(true);
  return 0;
}

static int cmd_detach(char *args) {
  difftest_toggle(false);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step execution for [N] steps", cmd_si },
  { "info", "Print program status\n"
            "info r - List of integer registers and their contents, for selected stack frame.\n"
            "info w - Status of specified watchpoints (all watchpoints if no argument).", cmd_info },
  { "p", "Evaluate an expression.", cmd_p },
  { "x", "Scan memeory. x [N] EXPR scan N QWORDs staring from address EXPR.", cmd_x },
  { "w", "Set watchpoint.", cmd_w },
  { "d", "Remove watchpoint", cmd_d },
  { "attach", "Turn on difftest.", cmd_attach },
  { "detach", "Turn off difftest.", cmd_detach },
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  
  uint64_t steps;
  if (arg == NULL) { // no arguments given
    steps = 1;
  } else {
    errno = 0;
    steps = strtoull(arg, NULL, 10);
    if (errno) { // an error ocurred
      Info("Invalid parameter: '%s'", arg);
      return 0;
    }
  }

  cpu_exec(steps);
  return 0;
}

static int cmd_info(char* args) {
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    Info("No subcmd provided.");
    return 0;
  } else if (strcmp(arg, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(arg, "w") == 0) {
    void watchpoint_list();
    watchpoint_list();
  } else {
    Info("Invalid subcmd.");
  }
  return 0;
}

static int cmd_x(char* args) {
  char* arg = strtok(NULL, " ");

  if (arg == NULL) {
    Info("No N provided.");
    return 0;
  }
  
  errno = 0;
  int N = strtol(arg, NULL, 10);
  if (errno) { // an error ocurred
    Info("Invalid parameter as N: '%s'", arg);
    return 0;
  }

  arg = strtok(NULL, "");
  if (arg == NULL) {
    Info("No EXPR provided.");
    return 0;
  }

  bool success = 1;
  paddr_t st_addr = expr(arg, &success);
  if (!success) {
    Info("Invalid EXPR.");
    return 0;
  }

  word_t paddr_read(paddr_t, int);
  while (N--) {
    printf("0x%x ", paddr_read(st_addr, 4));
    st_addr += 4;
  }
  printf("\n");

  return 0;  
}

static int cmd_w(char* args) {
  int id;
  int watchpoint_add(char *expression);
  if ((id = watchpoint_add(args)) == -1) {
    Info("Add watchpoint failed.");
    return 0;
  }
  Info("Watchpoint %d added: %s", id, args);
  return 0;
}

static int cmd_d(char* args) {
  int id = strtol(args, NULL, 10);
  errno = 0;
  if (errno) {
    Info("Invalid watchpoint id: '%s'", args);
    errno = 0;
    return 0;
  }
  void watchpoint_remove(int index);
  watchpoint_remove(id);
  return 0;
}


void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
