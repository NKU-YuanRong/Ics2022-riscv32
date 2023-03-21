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
  char expr[64];
  uint32_t value;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

// // get a new free watch point
// WP* new_wp();

// // free the watch point
// void free_wp(int N);

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

bool static Insert_to_head(WP *np) {
  if (!head) {
    head = np;
    np->next = NULL;
    return true;
  }
  WP* p = head;
  head = np;
  np->next = p;
  return true;
}

bool static Insert_to_free_(WP *np) {
  if (!free_) {
    free_ = np;
    np->next = NULL;
    return true;
  }
  WP* p = free_;
  free_ = np;
  np->next = p;
  return true;
}

// get a new free watch point
WP* new_wp() {
  if (!free_) {
    Log(ANSI_FMT("No free watch point!", ANSI_FG_RED));
    return NULL;
  }
  // Find first free node
  WP *p = free_;
  free_ = p->next;
  Insert_to_head(p);
  return p;
}

// free the watch point
void free_wp(int N) {
  if (!head) {
    Log(ANSI_FMT("No head watch point!", ANSI_FG_RED));
    return;
  }
  if (head->NO == N) {
    head = head->next;
    Insert_to_free_(wp_pool + N);
    return;
  }
  // Find first free node
  WP *p = head;
  WP *q = p->next;
  while (q) {
    if (q->NO == N) {
      p->next = q->next;
      Insert_to_free_(q);
      return;
    }
    p = q;
    q = q->next;
  }
  Log(ANSI_FMT("The watch point is free, no need to free it again!", ANSI_FG_RED));
  return;
}

bool cmd_new_wp(char *args, uint32_t val) {
  if (!args) {
    Log(ANSI_FMT("No expression!", ANSI_FG_RED));
    return false;
  }
  WP *p = new_wp();
  if (p == NULL) {
    return false;
  }
  strcpy(p->expr, args);
  p->value = val;
  Log("Get watch point %d on %s", p->NO, p->expr);
  return true;
}

bool difftest_watchpoint() {
  WP *p = head;
  bool suc, ex_stop = false;
  uint32_t val;
  while (p) {
    suc = false;
    val = expr(p->expr, &suc);
    assert(suc);
    if (val != p->value) {
      Log("Watch point %d is triggered, expr: %s", p->NO, p->expr);
      ex_stop = true;
    }
    p = p->next;
  }
  return ex_stop;
}

void show_all_wp() {
  if (!head) {
    printf("No watchpoints.\n");
    return;
  }
  WP *p = head;
  printf("Number\tType\t\tExpr\n");
  while (p) {
    printf("%d\thw watchpoint\t%s\n", p->NO, p->expr);
    p = p->next;
  }
}