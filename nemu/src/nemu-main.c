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

#include "macro.h"
#include <common.h>
#include <stdio.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

#define RUN_TESTS

#ifdef RUN_TESTS

int test_eval() {
  word_t expr(char *, bool *);

  bool success;
  FILE* file = fopen("tools/gen-expr/build/input", "r");
  if (!file) {
    fprintf(stderr, "Failed to open test file: input\n");
    return 1;
  }

  word_t ans, res;
  char buf[1048576];

  while (fscanf(file, "%u %[^\n]", &ans, buf) != EOF) {
    res = expr(buf, &success);
    if (res != ans) {
      fprintf(stderr, "Eval answer error: res: %u, ans: %u\n", res, ans);
    }
  }
  return 0;
}

struct {
  const char* name;
  int (*test_func)(void);
} tests [] = {
  {"Eval test", test_eval}
};

void run_all_tests() {
  int number_of_tests = ARRLEN(tests);

  for (int i = 0; i < number_of_tests; i ++) {
    fprintf(stderr, "\nRunning test: #%d: %s\n", i, tests[i].name);

    int ret = tests[i].test_func();

    fprintf(stderr, "\nTest #%d finished, ret: %d\n", i, ret);
    if (ret != 0) {
      fprintf(stderr, "Test failed!");
    }
  }
}

#endif

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef RUN_TESTS

#else
  /* Start engine. */
  engine_start();
#endif
  return is_exit_status_bad();
}
