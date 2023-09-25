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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expression;
  word_t value;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expression = NULL;
  }

  head = NULL;
  free_ = &wp_pool[0];
}

static WP* new_wp() {
  if (free_ == NULL) {
    panic("Watchpoint pool is full.");
  }

  WP* ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  return ret;
}

static bool free_wp(WP *wp) {
  if (head == NULL || wp == NULL) {
    goto error;
  }
  if (head == wp) {
    WP* tmp = head->next;
    head->next = free_;
    free_ = head;
    head = tmp;
    return true;
  }
  for (WP *cur = head; cur != NULL; cur = cur->next) {
    if (cur->next == wp) {
      WP* tmp = cur->next;
      cur->next = tmp->next;
      tmp->next = free_;
      free_ = tmp;
      break;
    }
  }
error: // fall through
  return false;
}

bool watchpoint_scan() {
  bool hit = 0;

  for (WP *cur = head; cur != NULL; cur = cur->next) {
    bool success = 0;
    word_t nvalue = expr(cur->expression, &success);
    if (nvalue != cur->value) {
      hit = 1;
      cur->value = nvalue;
      break;
    }
  }

  return hit;
}

int watchpoint_add(char *expression) {
  bool success = 0;
  word_t value = expr(expression, &success);
  if (!success) goto error;
  WP *wp = new_wp();
  wp->expression = strdup(expression);
  wp->value = value;
  return wp->NO;
error:
  Info("Invalid EXPR: %s", expression);
  return -1;
}

void watchpoint_remove(int index) {
  if (index < 0 || index >= NR_WP) goto error;

  if (wp_pool[index].expression != NULL) {
    free(wp_pool[index].expression);
  }
  if (!free_wp(&wp_pool[index])) goto error;

  return;

error:
  Error("No such watchpoint: %d", index);
}

void watchpoint_list() {
  Info("Id    Value         Expression");
  for (WP* cur = head; cur != NULL; cur = cur->next) {
    printf("%4d  %12u  %s\n", cur->NO, cur->value, cur->expression);
  }
}
