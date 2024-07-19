#include <func-trace.h>


static uint64_t call_depth = 0;

#define CALL_STACK_BUF_LENGTH 6
static uint8_t call_stack_buf_index = 0;
// 为了比较用的buf
static char call_stack_buf[CALL_STACK_BUF_LENGTH][128];
// 为了打印用的buf
static char call_stack_log_buf[CALL_STACK_BUF_LENGTH][128];
#define INIT_CALL_FROM  ""


#ifdef CONFIG_FTRACE
static void print_call_trace(){
    printf("%s\n", call_stack_log_buf[call_stack_buf_index]);
    // Log("%s\n", call_stack_log_buf[call_stack_buf_index]);
}

static void print_ret_trace(char* ret_from, char* ret_to, Decode *s){
    printf("0x%lx %lu ret from %s to %s\n", s->pc, call_depth, ret_from, ret_to);
    // Log("0x%lx %lu ret from %s to %s\n", s->pc, call_depth, ret_from, ret_to);
}

/*打印函数调用*/
static void print_jal_trace(){
    print_call_trace();
}
#endif

/* 增加一级调用链 */
void log_jal_stack(int rd, Decode *s){
    char* call_from = find_func_name(s->pc);
    char* call_to = find_func_name(s->dnpc);
    if (call_from && call_to && strcmp(call_from, call_to) == 0){
        // 函数内跳转，不算函数调用
        return;
    }
    if(call_from == NULL){
        call_from = INIT_CALL_FROM;
    }
    strcpy(call_stack_buf[call_stack_buf_index], call_from);
    sprintf(call_stack_log_buf[call_stack_buf_index], "0x%lx %lu call from %s to %s", s->pc, call_depth, call_from, call_to);
    IFDEF(CONFIG_FTRACE, print_jal_trace());
    call_depth += 1;
    call_stack_buf_index += 1;
    call_stack_buf_index %= CALL_STACK_BUF_LENGTH;

}

/* 从函数返回，减少一级调用链 */
void log_jalr_stack(int rd, Decode *s){
    char* from = find_func_name(s->pc);
    char* to = find_func_name(s->dnpc);
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    if (rd){
        log_jal_stack(rd, s);
    }else if (rs1 == 1){
        if (strcmp(from, to) == 0){
            // 函数内调用，不算返回
            return;
        }
        IFDEF(CONFIG_FTRACE, print_ret_trace(from, to, s));

        call_depth -= 1;
        if (call_stack_buf_index == 0){
            call_stack_buf_index = CALL_STACK_BUF_LENGTH - 1;
        }else{
            call_stack_buf_index -= 1;
        }
        
        for(int tmp_index = CALL_STACK_BUF_LENGTH; tmp_index > 0 && strcmp(call_stack_buf[call_stack_buf_index], to) != 0; tmp_index-= 1){
            // printf("last call from %s != %s, continue\n", call_stack_buf[call_stack_buf_index], to);
            call_depth -= 1;
            if (call_stack_buf_index == 0){
                call_stack_buf_index = CALL_STACK_BUF_LENGTH - 1;
            }else{
                call_stack_buf_index -= 1;
            }
        }
        
    }else{
        
        log_jal_stack(rd, s);
       
    }
    
}

/*打印调用链*/
void print_call_stack(){
    printf("call stack: \n");
    uint8_t tmp_call_stack_index = (call_stack_buf_index + 1) % CALL_STACK_BUF_LENGTH, i = 0, j=0;
    for( ; i<CALL_STACK_BUF_LENGTH; i++){
        for(j=0; j<i;j++){
            printf("  ");
        }
        printf("%s\n", call_stack_log_buf[tmp_call_stack_index]);
        tmp_call_stack_index ++;
        tmp_call_stack_index %= CALL_STACK_BUF_LENGTH;
  }
}