
#include <elf.h>
#include <common.h>

#include <isa.h>

void parse_img_elf(char *img_elf_file);
void finish_elf();
char * find_func_name(uint64_t inst_vaddr);
void print_call_trace(int dst, uint64_t dnpc, uint64_t pc);
void print_ret_trace(uint64_t pc);
void log_call_stack(int dst, uint64_t dnpc, uint64_t pc);
void log_ret();
void print_call_stack();