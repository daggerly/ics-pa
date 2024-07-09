#include <func-trace.h>


static uint64_t call_depth = 0;

#define CALL_STACK_BUF_LENGTH 6
static uint8_t call_stack_buf_index = 0;
static char call_stack_buf[CALL_STACK_BUF_LENGTH][128];


/*打印函数调用*/
void print_call_trace(int dst, uint64_t dnpc, uint64_t pc){
    if(dst){
        for(uint64_t tmp_ident=0;tmp_ident< call_depth; tmp_ident++){
          printf("   ");
        }
        printf("call %s @0x%lx\n", find_func_name(dnpc), pc);
        call_depth ++;
    }
}
 
/*打印函数返回*/
void print_ret_trace(uint64_t pc){
    for(uint64_t tmp_ident=0;tmp_ident< call_depth-1; tmp_ident++){
          printf("   ");
    }
    printf("ret %s@0x%lx\n", find_func_name(pc), pc);
    call_depth --;
}

/* 增加一级调用链 */
void log_call_stack(int dst, uint64_t dnpc, uint64_t pc){
    if(dst){
        sprintf(call_stack_buf[call_stack_buf_index], "call %s @0x%lx", find_func_name(dnpc), pc);
        call_stack_buf_index ++;
        call_stack_buf_index %= CALL_STACK_BUF_LENGTH;
    }
}

/* 从函数返回，减少一级调用链 */
void log_ret(){
    if (call_stack_buf_index == 0){
        call_stack_buf_index = CALL_STACK_BUF_LENGTH - 1;
    }else{
        call_stack_buf_index --;
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
        printf("%s\n", call_stack_buf[tmp_call_stack_index]);
        tmp_call_stack_index ++;
        tmp_call_stack_index %= CALL_STACK_BUF_LENGTH;
  }
}