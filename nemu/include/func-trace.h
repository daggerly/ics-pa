
#include <elf.h>
#include <common.h>

#include <isa.h>
#include <cpu/decode.h>

void parse_img_elf(char *img_elf_file);
void finish_elf();
char * find_func_name(uint64_t inst_vaddr);
void log_jal_stack(int dst, Decode *s);
void log_jalr_stack(int dst, Decode *s);
void print_call_stack();