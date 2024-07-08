
#include <elf.h>
#include <common.h>

#include <isa.h>

void parse_img_elf(char *img_elf_file);
void finish_elf();
char * find_func_name(uint64_t inst_vaddr);
