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

#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_LE,  // <=
  TK_REG,  // registry name
  TK_HEX,  // Hex number
  TK_NUM,  // number
  TK_DEREF,  // *p

  TK_AND,  // &&


};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\${1,2}[a-z]*[0-9]*", TK_REG},    // registry name
  {"0x[0-9a-f]+", TK_HEX},    // Hex number
  {"[0-9]+", TK_NUM},    // number
//  {"\\*\\([a-z0-9 \\+-\\*/\\$]+\\)", TK_DEREF},    // *($a0)取地址的值,如果使用该表达式，则括号前后不许有其他表达式
  {"\\*", '*'},    // 乘法
  {"-", '-'},      // 减法
  {"/", '/'},         // 除法
  {"\\(", '('},         // 左括号
  {"\\)", ')'},         // 右括号
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"<=", TK_LE},        // less or equal
  {"&&", TK_AND},        // and
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char *type_str;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

void init_tokens(){
  for (int i=0; i<sizeof(tokens)/sizeof(tokens[0]); i++){
    tokens[i].type = 0;
    for (uint64_t j=0;j<strlen(tokens[i].str); j++){
      tokens[i].str[j] = '\0';
    }
  }
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  init_tokens();
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_REG:
            tokens[nr_token].type= TK_REG;
            tokens[nr_token].type_str= "TK_REG";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_HEX:
            tokens[nr_token].type= TK_HEX;
            tokens[nr_token].type_str= "TK_HEX";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_NUM:
            tokens[nr_token].type= TK_NUM;
            tokens[nr_token].type_str= "TK_NUM";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case '+':
            tokens[nr_token].type= '+';
            tokens[nr_token].type_str= "+";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case '-':
            tokens[nr_token].type= '-';
            tokens[nr_token].type_str= "-";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case '*':
            if (
              (nr_token == 0) ||
              (tokens[nr_token-1].type == '+') ||
              (tokens[nr_token-1].type == '-') ||
              (tokens[nr_token-1].type == '*') ||
              (tokens[nr_token-1].type == '/')
            ){
              tokens[nr_token].type= TK_DEREF;
              tokens[nr_token].type_str= "TK_DEREF";
              memcpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0';
            } else{
              tokens[nr_token].type= '*';
              tokens[nr_token].type_str= "*";
              memcpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0';
            }
            nr_token += 1;
            break;
          case '/':
            tokens[nr_token].type= '/';
            tokens[nr_token].type_str= "/";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case '(':
            tokens[nr_token].type= '(';
            tokens[nr_token].type_str= "(";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case ')':
            tokens[nr_token].type= ')';
            tokens[nr_token].type_str= ")";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_EQ:
            tokens[nr_token].type= TK_EQ;
            tokens[nr_token].type_str= "==";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_LE:
            tokens[nr_token].type= TK_LE;
            tokens[nr_token].type_str= "<=";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_AND:
            tokens[nr_token].type= TK_AND;
            tokens[nr_token].type_str= "&&";
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token += 1;
            break;
          case TK_NOTYPE:
            // white space, drop it
            break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

void print_tokens(){
  for (int i=0; i<sizeof(tokens)/sizeof(tokens[0]); i++){
    if (tokens[i].type == 0) break;
    Log("token[%d] type: %s, str: %s", i, tokens[i].type_str, tokens[i].str);
  }
}

char * copy_subtoken_str(int p, int q){
  int n = 0, tmp_p = p;
  for(;p <= q; p+=1){
      n += strlen(tokens[p].str);
  }
  p = tmp_p;
  char * subtoken = malloc(n + 1);
  n = 0;
  for (; p<=q; p+=1){
    memcpy(subtoken+n, tokens[p].str, strlen(tokens[p].str));
    n += strlen(tokens[p].str);
  }
  *(subtoken+n) = '\0';
  return subtoken;
}

void print_subtoken_str(char * fmt, int p, int q){
  char * subtoken_str = copy_subtoken_str(p, q);
  printf(fmt, subtoken_str);
  free(subtoken_str);
}

bool check_parentheses(int p, int q, bool *good){
  print_subtoken_str("checking parentheses: %s\n", p, q);

  *good = true;
  bool parentheses_surrounded = tokens[p].str[0] == '(';
  if (parentheses_surrounded) {
    parentheses_surrounded = tokens[q].str[strlen(tokens[q].str)-1] == ')';
  }
  int left_num = 0;
  for(;p<=q;p++){
    int i = 0;
    for(;i<strlen(tokens[p].str);i++){
      if (tokens[p].str[i] == '('){
        left_num += 1;
      }
      else if (tokens[p].str[i] == ')'){
        left_num -= 1;
        if ((left_num == 0) && ((p < q) || (i < (strlen(tokens[q].str)-1)))
        ){
          parentheses_surrounded = false;
        } else if (left_num < 0){
          *good = false;
          printf("check_parentheses false false \n");
          return false;
        }
      }
    }
  }
  if ((left_num == 0) && parentheses_surrounded){}
  else parentheses_surrounded = false;
  Log("check_parentheses result: surround:%s, good expr: %s \n", bool2str(parentheses_surrounded), bool2str(*good));
  return parentheses_surrounded;
}

int find_main_op(int p, int q){
  int main_op_pos = -1;
  int left_parentheses_num = 0;
  for (;p<=q; p+=1){
    if (
    (tokens[p].type == TK_NUM) ||
    (tokens[p].type == TK_HEX) ||
    (tokens[p].type == TK_REG)
    ) {
      Log("number/depoint/reg not pos, now: %d %s\n", p, tokens[p].str);
      continue;
    }
    else if (tokens[p].type == '(') {left_parentheses_num += 1;
//      Log("left_parentheses_num += 1, now: %d %s\n", p, tokens[p].str);
    }
    else if (tokens[p].type == ')') {left_parentheses_num -= 1;
//      Log("left_parentheses_num -= 1, now: %d %s\n", p, tokens[p].str);
    }
    else if (left_parentheses_num > 0) {
      Log("find %s, but in parentheses, now: %d %s\n", tokens[p].str, p, tokens[p].str);
      continue;
    }
    else if ((tokens[p].type == '*') || (tokens[p].type == '/')){
      if ((main_op_pos == -1) ||
        ((tokens[main_op_pos].type == '*') || (tokens[main_op_pos].type == '/'))
      ) main_op_pos = p;
      Log("find */ , pos: %d, now: %d %s\n", main_op_pos, p, tokens[p].str);
    }
    else if ((tokens[p].type == '+') || (tokens[p].type == '-')){
      main_op_pos = p;
      Log("find +- , pos: %d, now: %d %s\n", main_op_pos, p, tokens[p].str);
    }
    else if (tokens[p].type == TK_DEREF){
      if ((main_op_pos == -1) || (tokens[main_op_pos].type == TK_DEREF))
      main_op_pos = p;
      Log("find DEREF , pos: %d, now: %d %s\n", main_op_pos, p, tokens[p].str);
    }

    else{
      Log("now: %d %s\n", p, tokens[p].str);
      TODO();
    }
  }
  return main_op_pos;
}

word_t eval(int p, int q, bool *success) {
  char * subtoken_str = copy_subtoken_str(p, q);

  word_t result = 0;
  bool good_expr = true;
  if (p > q) {
    if (tokens[p].type == TK_DEREF){
      *success = true;
    }
    else{
      *success = false;
      Warning("bad token index %d %d\n", p, q);
    }
    free(subtoken_str);
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
     if (tokens[p].type == TK_HEX){
       char *endptr;
       result = strtol(tokens[p].str, &endptr, 16);
     }
     else if (tokens[p].type == TK_REG){
       result = isa_reg_str2val(tokens[p].str+1, success);
       if (!*success) {
         free(subtoken_str);
         return 0;
       }
    }
    else{
       result = atoi(tokens[p].str);
     }
  }
  else if (check_parentheses(p, q, &good_expr) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    result = eval(p + 1, q - 1, success);
  }
  else if (!good_expr){
    *success = false;
    Warning("bad expr  %s\n", subtoken_str);
    free(subtoken_str);
    return 0;
  }

  else {
    int op = find_main_op(p, q);
    if (op < 0){
      Warning("bad expr %s\n", subtoken_str);
      free(subtoken_str);
      *success = false;
      return 0;
    }
    Log("op %s\n", tokens[op].str);
    word_t val1 = eval(p, op - 1, success);
    if (!*success) {
      free(subtoken_str);
      return 0;
    }
    word_t val2 = eval(op + 1, q, success);
    printf("%08lx", val2);
    if (!*success) {
      free(subtoken_str);
      return 0;
    }

    switch (tokens[op].type) {
      case '+':
        result = val1 + val2;
        break;
      case '-':
        result = val1 - val2;
        break;
      case '*':
        result = val1 * val2;
        break;
      case '/':
        result = val1 / val2;
        break;
      case TK_DEREF:
        if (in_pmem(val2)){
          result = vaddr_read(val2, 4);
        }
        else{
          *success = false;
          Warning("%s = 0x%08lx not in pmem\n", subtoken_str, val2);
          free(subtoken_str);
          return 0;
        }
        break;
      default:
        free(subtoken_str);
        assert(0);
    }
  }
  Log("p: %d, q: %d, eval: %s, result: 0x%08lx\n", p, q, subtoken_str, result);
  free(subtoken_str);
  return result;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  print_tokens();

  /* TODO: Insert codes to evaluate the expression. */
  int p = 0, q;
  for(q = 0;tokens[q].type >0; q++){}

  return eval(p, q - 1, success);
}
