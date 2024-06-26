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

#include <common.h>

extern uint64_t g_nr_guest_inst;
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}


void print_int32_binary(char* name, int num) {
    printf("%s 0x%x %d 0b", name, num, num);
    bool first_valid = false;
    for (int i = sizeof(int)*8 - 1; i >= 0; i--) {
        // 右移位运算符将数字右移i位
        int bit = (num >> i) & 1;
        if (bit){
          first_valid = true;
        }
        if (!first_valid) continue;
        printf("%d", bit);
    }
    if (!first_valid) printf("0");
    printf("\n");
}

void print_u32_binary(char* name, uint32_t num) {
    printf("%s 0x%x %u 0b", name, num, num);
    bool first_valid = false;
    for (int i = sizeof(int)*8 - 1; i >= 0; i--) {
        // 右移位运算符将数字右移i位
        int bit = (num >> i) & 1;
        if (bit){
          first_valid = true;
        }
        if (!first_valid) continue;
        printf("%d", bit);
    }
    if (!first_valid) printf("0");
    printf("\n");
}

void print_u64_binary(char* name, word_t num) {
    printf("%s 0x%lx %lu 0b", name, num, num);
    bool first_valid = false;
    for (int i = sizeof(word_t)*8 - 1; i >= 0; i--) {
        // 右移位运算符将数字右移i位
        int bit = (num >> i) & 1;
        if (bit){
          first_valid = true;
        }
        if (!first_valid) continue;
        printf("%d", bit);
    }
    if (!first_valid) printf("0");
    printf("\n");
}

void print_int64_binary(char* name, int64_t num) {
    printf("%s 0x%lx %ld 0b", name, num, num);
    bool first_valid = false;
    for (int i = sizeof(int64_t)*8 - 1; i >= 0; i--) {
        // 右移位运算符将数字右移i位
        int bit = (num >> i) & 1;
        if (bit){
          first_valid = true;
        }
        if (!first_valid) continue;
        printf("%d", bit);
    }
    if (!first_valid) printf("0");
    printf("\n");
}