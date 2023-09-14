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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[1048576] = {};
static char code_buf[1048576 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static const int MAX_DEPTH = 10;

int pt;

static void gen(char ch) {
  buf[pt ++] = ch;
}

static void gen_num() {
  unsigned int num = rand();

  if (num == 0) {
    buf[pt ++] = '0';
    return;
  }

  char n_buf[16] = {0};
  int cur = 0;
  while (num > 0) {
    n_buf[cur ++] = num % 10 + '0';
    num /= 10;
  }

  while (cur > 0) {
    buf[pt ++] = n_buf[-- cur];
  }
}

static void gen_rand_op() {
  char choice = rand() % 4;
  
  switch (choice) {
    case 0: choice = '+'; break;
    case 1: choice = '-'; break;
    case 2: choice = '*'; break;
    case 3: choice = '/'; break;
    default: choice = '+'; break;
  }

  buf[pt ++] = choice;
}

static void gen_rand_expr(int depth) {
  if (depth == MAX_DEPTH) {
    gen_num();
    return;
  }
  int choice = rand() % 3;
  switch (choice) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(depth + 1); gen(')'); break;
    default: gen_rand_expr(depth + 1); gen_rand_op(); gen_rand_expr(depth + 1); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    pt = 0;
    gen_rand_expr(0);
    buf[pt] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Wall -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {
      -- i;
      continue;
    }

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
