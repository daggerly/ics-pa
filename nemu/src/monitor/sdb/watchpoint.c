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

enum {
  WP_FREE = 1,
  WP_INUSE,

};


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char * expr;
  word_t val;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].expr = NULL;
    wp_pool[i].val = 0;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


void print_watchpoint(){
  printf("head: \n");
  WP *tmp = head;
  while(tmp){
    if (tmp->next) printf("  NO: %d, expr: %s, val: 0x%08lx (or %ld), next: %d\n",
      tmp->NO, tmp->expr, tmp->val, tmp->val, tmp->next->NO);
    else printf("  NO: %d, expr: %s, val: 0x%08lx (or %ld), END\n", tmp->NO, tmp->expr, tmp->val, tmp->val);
    tmp = tmp->next;
  }
  printf("free: \n");
  tmp = free_;
  while(tmp){
    if (tmp->next) printf("  NO: %d, next: %d\n", tmp->NO,tmp->next->NO);
    else printf("  NO: %d, END\n", tmp->NO);
    tmp = tmp->next;
  }
}


void _free_wp(WP *wp){
  if ((wp == NULL) || (head == NULL)){
    return;
  }
  free(wp->expr);
  WP * tmp = head, *pre = NULL;
  while (tmp && (tmp != wp)) {
    pre = tmp;
    tmp = tmp->next;
  }

  // 没找到
  if (tmp == NULL) return;
  // 要删除的就是head节点
  if (tmp == head) head = NULL;
  //
  if (pre) {pre->next = tmp->next;}

  if (free_ == NULL) free_ = wp;
  else{
    tmp = free_;
    while (tmp && tmp->next) {
      tmp = tmp->next;
    }
    tmp->next = wp;

  }
  wp->next = NULL;

  printf("删除watchpoint NO:%d\n", wp->NO);
  print_watchpoint();
  return;
}

void free_watchpoint(int nr){
  _free_wp(&wp_pool[nr]);
}

WP *_new_wp(){
  if (free_ == NULL){
    return NULL;
  }
  WP *wp = free_, *tmp=head;

  free_ = free_->next;
  wp->next = NULL;
  if (head == NULL){
    head = wp;
  }
  else{
    while(tmp->next){
      tmp = tmp->next;
    }
    tmp->next = wp;
  }

  return wp;
}

bool new_watchpoint(char *expr_str){
    WP * wp = _new_wp();
    if (!wp) return false;

    char * expr_copy = malloc(strlen(expr_str) + 1);
    memcpy(expr_copy, expr_str, strlen(expr_str));
    *(expr_copy + strlen(expr_str)) = '\0';
    wp->expr = expr_copy;
    bool success;
    word_t val = expr(expr_copy, &success);
    if (!success) {
      _free_wp(wp);
      return false;
    }
    wp->val = val;
    printf("增加watchpoint NO:%d\n", wp->NO);
    print_watchpoint();
    return true;


}

bool update_watchpoint_value(){
  WP *tmp = head;
  bool changed = false, success = false;
  while(tmp){
    word_t cur_val = expr(tmp->expr, &success);
    if (success){
      if (cur_val != tmp->val) {
        tmp->val = cur_val;
        changed = true;
      }
    }
    else{
      Warning("watchpoint %d eval expr %s failed", tmp->NO, tmp->expr);
    }
    tmp = tmp->next;
  }
  return changed;
}
