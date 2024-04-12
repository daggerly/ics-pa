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
#include <stdlib.h>
#include <errno.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
bool new_watchpoint(char *expr);
void free_watchpoint(int nr);
void print_watchpoint();

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
  return -1;
}

static int cmd_help(char *args);


static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  uint64_t n = 1;
  if (arg != NULL) {
    n = atoi(arg);
  }
  cpu_exec(n);
  return 0;
}


static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL){
    printf("bad arg, in r w\n");
    return 1;
  } else if ((*arg) == 'r'){
    isa_reg_display();
    return 0;
  } else if ((*arg) == 'w'){
    print_watchpoint();
    return 0;
  } else{
    printf("bad arg, in r w\n");
    return 1;
  }

}


static int cmd_scan(char *args) {

  char * args_all = malloc(strlen(args) + 1);
  memcpy(args_all, args, strlen(args));
  *(args_all + strlen(args)) = '\0';

  char *arg = strtok(NULL, " ");
  uint64_t n = 0;
  n = atoi(arg);
  if (n <= 0){
    free(args_all);
    Warning("bad N, must be > 0\n");
    return 0;
  }
  char * new_args_all = args_all + strlen(arg) + 1;
  bool success = true;
  word_t vaddr = expr(new_args_all, &success);
  if (!success){
    Warning("bad expr\n");
    free(args_all);
    return 0;
  }
  Log("expr %s -> vaddr: 0x%08lx", arg, vaddr);
  uint64_t i = 0;
  for (;i<n;i++){
    word_t v = vaddr+i*4;
    if (in_pmem(v)){
      printf("0x%08lx 0x%08lx\n", v, vaddr_read(v, 4));
    } else{
      printf("0x%08lx not in pmem\n", v);
    }
  }
  free(args_all);

return 0;
}

static int cmd_eval_expr(char *args){
  bool success = true;
  word_t val = expr(args, &success);
  if (!success){
    Warning("bad expr\n");
    return 0;
  }
  printf("%ld\n", val);
  return 0;
}

static int cmd_add_monitor(char *args){
  bool success = new_watchpoint(args);
  if (!success){
    Warning("add monitor failed\n");
    return 0;
  }
  return 0;
}

static int cmd_delete_monitor(char *args){
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    Warning("bad N, must be >= 0\n");
    return 0;
  }
  errno = 0;
  int n = atoi(arg);;

   if (errno != 0) {
    Warning("bad N, must be >= 0\n");
    return 0;
   }
  printf("%d", n);
  free_watchpoint(n);
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


  { "si", "step instruction", cmd_si },
  { "info", "print info\n  info r: print registry\n  info w: print monitor point", cmd_info },
  { "x", "scan memory\n  x N $reg", cmd_scan },
  { "p", "eval expr \n  p expr", cmd_eval_expr },
  { "w", "add monitor \n  w expr", cmd_add_monitor },
  { "d", "delete monitor \n  d N", cmd_delete_monitor },
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
