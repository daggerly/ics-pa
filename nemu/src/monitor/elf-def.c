
#include <elf.h>
#include <isa.h>

// 函数符号条目数量
static uint64_t func_sym_num = 0;

typedef struct
{
  char* name;		/* Symbol name */
  uint64_t	addr;		/* Func Start */
  uint64_t	end;		/* Func End */
} SymInfo;

SymInfo* SymInfoArray;

// /*打印 Section header结构信息*/
// static void print_section_header(Elf64_Shdr *section_header, void * section_name_section_data){
//     Log("section header info:\n"
//         "section name %s \n"
//         "section type %u \n"
//         "section flags %lu \n"
//         "section addr 0x%lx, \n"
//         "section offset 0x%lx, \n"
//         "section size %lu, \n"
//         "section link %u, \n"
//         "section info %u, \n"
//         "section align %lu, \n"
//         "section entsize %lu, \n",
//         (char *)(section_name_section_data + section_header->sh_name),
//         section_header->sh_type,
//         section_header->sh_flags,
//         section_header->sh_addr,
//         section_header->sh_offset,
//         section_header->sh_size,
//         section_header->sh_link,
//         section_header->sh_info,
//         section_header->sh_addralign,
//         section_header->sh_entsize);
// }


/* 查找一条指令所属函数名称 */
char * find_func_name(uint64_t inst_vaddr){
    for(uint64_t i=0; i<func_sym_num; i++ ){
        if((SymInfoArray[i].addr <= inst_vaddr) && (inst_vaddr < SymInfoArray[i].end)){
            // printf("find_func_name %s\n", SymInfoArray[i].name);
            return SymInfoArray[i].name;
        }
    }
    return NULL;
}

/* 解析一个ELF文件*/
void parse_img_elf(char *image_elf_file){
    void *elf_buf = NULL, 
         // 符号表基地址
         *sym_table_base = NULL,
         // 符号名字表基地址
         *string_table_base = NULL;
    // 符号条目大小
    const uint64_t symbol_entry_size = sizeof(Elf64_Sym);

    if (image_elf_file == NULL){
        Log("not set ELF file %c", '\0');
        return;
    }
    FILE *fp = fopen(image_elf_file, "rb");
    Assert(fp, "Can not open ELF file: '%s'", image_elf_file);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    Log("The ELF file is %s, size = %ld", image_elf_file, size);

    elf_buf = malloc(size);
    fseek(fp, 0, SEEK_SET);
    int ret = fread(elf_buf, size, 1, fp);
    assert(ret == 1);

    fclose(fp);

    Elf64_Ehdr *header = elf_buf;
    uint64_t elf_header_size = header->e_ehsize;
    uint64_t program_header_table_offset = header->e_phoff;
    uint64_t program_header_table_entry_count = header->e_phnum;
    uint64_t program_header_table_entry_size = header->e_phentsize;
    if (program_header_table_offset == 0){
        program_header_table_entry_count = program_header_table_entry_size = 0;
    }

    uint64_t section_header_table_offset = header->e_shoff;
    Elf64_Shdr *initial_section_header = elf_buf + section_header_table_offset;
    uint64_t section_header_table_entry_size = header->e_shentsize;
    uint64_t section_header_table_entry_count = header->e_shnum;
    /* If the file has no section name string table, this member
              holds the value SHN_UNDEF.*/
    uint64_t section_header_string_table_index = header->e_shstrndx;
    if (section_header_table_offset == 0){
        section_header_table_entry_size = section_header_table_entry_count = 0;
        section_header_string_table_index = SHN_UNDEF;
    }else if (section_header_table_entry_count == 0){
        /*   If the number of entries in the section header table is
              larger than or equal to SHN_LORESERVE (0xff00), e_shnum
              holds the value zero and the real number of entries in the
              section header table is held in the sh_size member of the
              initial entry in section header table. */
        section_header_table_entry_count = initial_section_header->sh_size;
    }

    if (program_header_table_entry_count == PN_XNUM){
        /*the real number of entries in the program header table is held in the sh_info member of 
          the initial entry in section header table*/
        program_header_table_entry_count = initial_section_header->sh_info;
    }

    uint64_t program_header_table_size = program_header_table_entry_count * program_header_table_entry_size;
    uint64_t section_header_table_size = section_header_table_entry_count * section_header_table_entry_size;
    Log("ELF file info:\n"
        "elf_header_size %lu ,\n"
        "program_header_table_offset 0x%lx ,\n"
        "program_header_table_entry_count %lu, \n"
        "program_header_table_entry_size %lu, \n"
        "program_header_table_size %lu, \n"
        "section_header_table_offset 0x%lx, \n"
        "section_header_table_entry_size %lu, \n"
        "section_header_table_entry_count %lu, \n"
        "section_header_table_size %lu, \n"
        "section_header_string_table_index %lu, \n",
        elf_header_size,
        program_header_table_offset,
        program_header_table_entry_count,
        program_header_table_entry_size,
        program_header_table_size,
        section_header_table_offset,
        section_header_table_entry_size,
        section_header_table_entry_count,
        section_header_table_size,
        section_header_string_table_index);

    /* 循环用 */
    uint64_t i = 0, symbol_num = 0;

    for(i = 0; i< section_header_table_entry_count; i++){
        Elf64_Shdr *section_header = elf_buf + section_header_table_offset + i*section_header_table_entry_size;
        if (section_header->sh_type == SHT_SYMTAB){
            sym_table_base = elf_buf + section_header->sh_offset;
            symbol_num = section_header->sh_size / section_header->sh_entsize;
            uint32_t sh_link = section_header->sh_link;
            Elf64_Shdr *str_section_header = elf_buf + section_header_table_offset + sh_link*section_header_table_entry_size;
            string_table_base = elf_buf + str_section_header->sh_offset;
            break;
        }
        
    }
    
    Assert(sym_table_base, "无法解析符号表\n");
    Assert(string_table_base, "无法解析字符表\n");

    for(i=0; i<symbol_num; i++){
        Elf64_Sym *sym = sym_table_base + i*symbol_entry_size;
        if ((sym->st_name == 0) || ELF64_ST_TYPE(sym->st_info) != STT_FUNC) continue;
        func_sym_num ++;
    }
   
    SymInfoArray = (SymInfo*)malloc(sizeof(SymInfo) * func_sym_num);
    Assert(SymInfoArray, "分配内存失败");

    uint64_t tmp_func_sym_num = 0;
    for(i=0; i<symbol_num; i++){
        Elf64_Sym *sym = sym_table_base + i*symbol_entry_size;
        if ((sym->st_name == 0) || ELF64_ST_TYPE(sym->st_info) != STT_FUNC) continue;

        // printf("%s %u\n", (char *)(string_table_base + sym->st_name), ELF64_ST_TYPE(sym->st_info));
        
        // set name
        int name_length = strlen(string_table_base + sym->st_name);
        char* sym_name = (char*)malloc((name_length+1)*sizeof(char));
        Assert(sym_name, "分配内存失败");
        strcpy(sym_name, (char*)(string_table_base + sym->st_name));
        SymInfoArray[tmp_func_sym_num].name = sym_name;
        SymInfoArray[tmp_func_sym_num].addr = sym->st_value;
        SymInfoArray[tmp_func_sym_num].end = sym->st_value + sym->st_size;

        tmp_func_sym_num ++;
    }
    free(elf_buf);
}

void finish_elf(){
    if (SymInfoArray){
        for(uint64_t i=0;i<func_sym_num;i++){
            free(SymInfoArray[i].name);
        }
        free(SymInfoArray);
    }
}