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

#include "local-include/reg.h"
#include <debug.h>
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <func-trace.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_RR, TYPE_B, TYPE_J,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = \
                    (SEXT(BITS(i, 31, 31), 1) << 20) | \
                    BITS(i, 30, 21) << 1 | \
                    BITS(i, 20, 20) << 11 | \
                    BITS(i, 19, 12) << 12;  \
                  } while(0)
#define immB() do { *imm = \
                    (SEXT(BITS(i, 31, 31), 1) << 12) | \
                    BITS(i, 30, 25) << 5 | \
                    BITS(i, 11, 8) << 1 | \
                    BITS(i, 7, 7) << 11;  \
                  } while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_RR: src1R(); src2R();        break;
    case TYPE_B:  src1R(); src2R();immB(); break;
    case TYPE_J:                   immJ(); break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  int64_t int64_src1 = 0, int64_src2 = 0;
  uint32_t uint32_src1 = 0, uint32_src2 = 0;
  int32_t int32_src1 = 0,  int32_src2 = 0;
  s->dnpc = s->snpc;
  // Log("execing: 0x%lx", s->pc);
  // word_t addr = 0x800004a8;
  // for(int p_i=0;p_i<13;p_i++){
  //   word_t p_c = vaddr_read(addr + p_i, 1);
  //   Log("execing: %c %lu", (int)p_c, p_c);
  // }
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}
#define LOG_OPERAND() {print_u64_binary("src1", src1); print_u64_binary("src2", src2); print_u64_binary("rd", rd); print_u64_binary("imm", imm);}
  

  INSTPAT_START();
  //             imm              | rd |  opcode
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  //           imm| rs1 |funct3   | rd  | opcode
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld     , I, R(rd) = Mr(src1 + imm, 8));
    //       imm  | rs1 |funct3|  rd | opcode
  INSTPAT("??????? ????? ????? 010    ????? 00000 11", lw     , I, 
    R(rd) = SEXT(Mr(src1 + imm, 4), 32);
    // Log("lw read %lx from %lx", R(rd), src1 + imm);
  );
  // 从内存加载一个16位的值 符号扩展到64位
  INSTPAT("???????????? ????? 001 ????? 0000011", lh      , I, 
    R(rd) = SEXT(Mr(src1 + imm, 2), 16);
    // Log("lh read %lx from %lx", R(rd), src1 + imm);
  );
  // 从内存加载一个16位的值 0扩展到64位
  INSTPAT("???????????? ????? 101 ????? 0000011", lhu      , I, 
    R(rd) = Mr(src1 + imm, 2);
    // Log("lhu read %lx from %lx", R(rd), src1 + imm);
  );
  // 从内存加载一个8位的值 0扩展到64位
  //          imm      | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 100 ????? 0000011", lbu      , I, 
    // isa_reg_display();
    // print_u64_binary("mr", src1 + imm);
    // Log("%lx", vaddr_read(src1 + imm, 1));
    R(rd) = Mr(src1 + imm, 1);
    // isa_reg_display();
  );

  // 写入32位数据到rs1+imm
  //        imm   | rs2 | rs1 |funct3|imm| opcode
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 8, src2));
  // 写入32位数据到rs1+imm
  //        imm   | rs2 | rs1 |funct3|imm  | opcode
  INSTPAT("??????? ????? ?????  010   ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  // 写入16位数据到rs1+imm
  //        imm   | rs2 | rs1 |funct3|imm  | opcode
  INSTPAT("??????? ????? ?????  001   ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));

  // 写入8位数据到rs1+imm
  //        imm   | rs2 | rs1 |funct3|imm  | opcode
  INSTPAT("??????? ????? ?????  000   ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  //             imm   | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 000 ????? 00100 11", addi      , I,  
    R(rd) = src1 + imm;
    // Log("addi: %s = 0x%lx + 0x%lx = 0x%lx", reg_name(rd, sizeof(word_t)), src1, imm, R(rd));
  );
  //             imm   | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 111 ????? 00100 11", andi      , I, 
    R(rd) = src1 & imm;
    // Log("andi: %s set to %lu", reg_name(rd, sizeof(word_t)), src1 & imm);
  );
  //             imm   | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 000 ????? 00110 11", addiw      , I, 
    // isa_reg_display();
    // LOG_OPERAND();
    uint32_src1 = src1 + SEXT(imm, 12);
    R(rd) = SEXT(uint32_src1, 32);
    // Log("addiw: %s = 0x%lx + 0x%lx = 0x%x extend to 0x%lx", reg_name(rd, sizeof(word_t)), src1, SEXT(imm, 12), uint32_src1, R(rd));
  );
  // R(rs1)与有符号imm异或
  //             imm   | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 100 ????? 00100 11", xori      , I, 
    R(rd) = src1 ^ SEXT(imm, 12);
    // Log("xori: %s = 0x%lx ^ 0x%lx = 0x%lx", reg_name(rd, sizeof(word_t)), src1, SEXT(imm, 12), R(rd));
  );
  // 64位算术右移
  //               shamt| rs1 |   | rd  | opcode
  INSTPAT("0100000 ????? ????? 101 ????? 0010011", srai      , I, 
    imm &= 0b11111;
    int64_src1 = (int64_t)src1;
    int64_src1 = int64_src1 >> imm;
    R(rd) = (word_t)int64_src1;
    // Log("srai: %lx right shift %lu to %lx, save into %s", src1, imm, int64_src1, reg_name(rd, sizeof(word_t)));
  );
  // 32位算术右移(带符号右移)
  INSTPAT("0100000 ????? ????? 101 ????? 0011011", sraiw      , I, 
    imm = imm & 0b11111;
    int32_src1 = src1;
    int32_src2 = int32_src1 >> imm;
    src1 = int32_src2;
    R(rd) = src1;
    // Log("slaiw: 0x%x right shift %lu to 0x%lx, save into %s", int32_src1, imm, R(rd), reg_name(rd, sizeof(word_t)));
  );
  // 32位算术右移(带符号右移)
  INSTPAT("0100000 ????? ????? 101 ????? 0111011", sraw      , RR, 
    src2 = src2 & 0b11111;
    int32_src1 = src1;
    int32_src2 = int32_src1 >> src2;
    src1 = int32_src2;
    R(rd) = src1;
    // Log("sraw: 0x%x right shift %lu to 0x%lx, save into %s", int32_src1, src2, R(rd), reg_name(rd, sizeof(word_t)));
  );
  // 逻辑左移
  // TODO bubble-sort中8000003c:	02059793 slli	a5,a1,0x20的译码为
  //         0000001 00000 01011 001 01111 0010011
  // 而手册上是0000000 ????? ????? 001 ????? 0010011
  // 不符合手册，不知道为什么
  //               shamt| rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 001 ????? 0010011", slli      , I, 
    R(rd) = src1 << imm;
    // Log("slli: 0x%lx left shift %lu to 0x%lx, save into %s", src1, imm, R(rd), reg_name(rd, sizeof(word_t)));
  );
  // 32位逻辑左移
  INSTPAT("0000000 ????? ????? 001 ????? 0011011", slliw      , I, 
    imm = imm & 0b11111;
    uint32_src1 = src1;
    uint32_src2 = uint32_src1 << imm;
    R(rd) = SEXT(uint32_src2, 32);
    // Log("slliw: 0x%x left shift %lu to 0x%lx, save into %s", uint32_src1, imm, R(rd), reg_name(rd, sizeof(word_t)));
  );
  // 64位逻辑右移
  // TODO bubble-sort中80000040:	01e7d613  srli	a2,a5,0x1e的译码为
  //         0000001 01110 01111 101 01100 0010011
  // 而手册上是0000000 ????? ????? 101 ????? 0010011
  // 不符合手册，不知道为什么
  //               shamt| rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 101 ????? 0010011", srli      , I, 
    R(rd) = src1 >> imm;
    // Log("srli: 0x%lx right shift %lu to 0x%lx, save into %s", src1, imm, src1 >> imm, reg_name(rd, sizeof(word_t)));
  );
  // 32位逻辑右移(无符号右移),根据立即数
  INSTPAT("0000000 ????? ????? 101 ????? 0011011", srliw      , I, 
    imm = imm & 0b11111;
    uint32_src1 = src1;
    uint32_src2 = uint32_src1 >> imm;
    src1 = uint32_src2;
    R(rd) = src1;
    // Log("srliw: 0x%x right shift %lu to 0x%lx, save into %s", uint32_src1, imm, src1, reg_name(rd, sizeof(word_t)));
  );
  // 32位逻辑右移(无符号右移)，根据寄存器内数
  INSTPAT("0000000 ????? ????? 101 ????? 0111011", srlw      , RR, 
    src2 = src2 & 0b11111;
    uint32_src1 = src1;
    uint32_src2 = uint32_src1 >> src2;
    src1 = uint32_src2;
    R(rd) = src1;
    // Log("srlw: 0x%x right shift %lu to 0x%lx, save into %s", uint32_src1, src2, src1, reg_name(rd, sizeof(word_t)));
  );
  //             imm           | rd  | opcode
  INSTPAT("???????????????????? ????? 01101 11", lui          , U, R(rd) = imm); 

  // imm[20]|imm[10:1] |imm[11]|imm[19:12]| rd  |opcode
  INSTPAT("? ??????????    ?    ????????   ????? 11011 11", jal, J, 
    R(rd) = s->snpc;
    s->dnpc = s->pc + imm;
    log_jal_stack(rd, s);
  );
  //             imm   | rs1 |funct3| rd  |  opcode
  INSTPAT("???????????? ????? 000    ????? 11001 11", jalr, I, 
    R(rd) = s->snpc;
    s->dnpc = (src1 + imm) & ~1;
    
    log_jalr_stack(rd, s);
  );
  // imm[12]|imm[10:5]| rs2 | rs1 |   |imm[4:1]|imm[11]| opcode
  // if (rs1 <s rs2) pc += sext(offset)
  INSTPAT("? ??????    ????? ????? 100 ????     ?       11000 11", blt, B, 
    int64_src1 = (int64_t)src1;
    int64_src2 = (int64_t)src2;
    if (int64_src1 < int64_src2) {
      s->dnpc = s->pc + imm;
    } 
    // if (int64_src1 < int64_src2) {
    //   Log("&0x%lx blt: src1: %ld < src2:%ld, dnpc = pc(%lx) + %lx ", s->pc, int64_src1, int64_src2, s->pc, imm);
    // } else{
    //   Log("&0x%lx blt: src1: %ld >= src2:%ld", s->pc, int64_src1, int64_src2);
    // }
  );
  // 无符号比较
  // imm[12]|imm[10:5]| rs2 | rs1 |   |imm[4:1]|imm[11]| opcode
  // if (rs1 < rs2) pc += sext(offset)
  INSTPAT("? ??????    ????? ????? 110 ????     ?       11000 11", bltu, B, 
    if (src1 < src2) {
      s->dnpc = s->pc + imm;
    } 
    // if (src1 < src2) {
    //   Log("bltu: src1: 0x%lx < src2:0x%lx, dnpc(0x%lx) = pc(0x%lx) + 0x%lx ", src1, src2, s->dnpc, s->pc, imm);
    // } else{
    //   Log("bltu: src1: 0x%ld >= src2:0x%ld", src1, src2);
    // }
  );
  // if (rs1 >=s rs2) pc += sext(offset)
  INSTPAT("? ??????    ????? ????? 101 ????     ?       11000 11", bge, B, 
    int64_src1 = (int64_t)src1;
    int64_src2 = (int64_t)src2;
    if (int64_src1 >= int64_src2) {
      s->dnpc = s->pc + imm;
    } 
    // if (int64_src1 >= int64_src2) {
    //   Log("&0x%lx bge: src1: %ld >= src2:%ld, dnpc = pc(%lx) + 0x%lx ", s->pc, int64_src1, int64_src2, s->pc, imm);
    // } else{
    //   Log("&0x%lx bge: src1: %ld < src2:%ld", s->pc, int64_src1, int64_src2);
    // }
  );
  // if (rs1 >= rs2) pc += sext(offset)
  INSTPAT("? ??????    ????? ????? 111 ????     ?       11000 11", bgeu, B, 
    if (src1 >= src2) {
      s->dnpc = s->pc + imm;
    } 
    // if (int64_src1 >= int64_src2) {
    //   Log("&0x%lx bge: src1: %ld >= src2:%ld, dnpc = pc(%lx) + 0x%lx ", s->pc, int64_src1, int64_src2, s->pc, imm);
    // } else{
    //   Log("&0x%lx bge: src1: %ld < src2:%ld", s->pc, int64_src1, int64_src2);
    // }
  );
  
  // 相等则跳转
  // imm[12]|imm[10:5]| rs2 | rs1 |   |imm[4:1]|imm[11]| opcode
  INSTPAT("? ??????    ????? ????? 000 ????        ?     11000 11", beq, B, 
    if (src1 == src2) {
      s->dnpc = s->pc + imm;
    }
  );

  // 不相等则跳转
  // imm[12]|imm[10:5]| rs2 | rs1 |   |imm[4:1]|imm[11]| opcode
  INSTPAT("? ??????    ????? ????? 001 ????        ?     11000 11", bne, B, 
    if (src1 != src2) {
      s->dnpc = s->pc + imm;
    }
    // if (src1 != src2) {
    //   Log("&0x%lx bne: src1: %lx != src2:%lx, dnpc = pc(%lx) + %lx = %lx ", s->pc, src1, src2, s->pc, imm, s->dnpc);
    // } else{
    //   Log("&0x%lx bne: src1: %lx == src2:%lx", s->pc, src1, src2);
    // }
  );
  // 64位加法
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, RR, 
    R(rd) = src1 + src2;
  // Log("add &0x%08lx: R(rd)(%s) set to src1: %lx + src2:%lx = %lx ", s->pc, reg_name(rd, sizeof(word_t)), src1, src2, src1 + src2);
  );

  // 32位加法，结果符号扩展成64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw, RR, 
    src1 = (uint32_t)src1;
    src2 = (uint32_t)src2;
    R(rd) = SEXT(src1 + src2, 32);
    // Log("addw &0x%08lx: R(rd)(%s) set to src1: %lx + src2:%lx = %lx ", s->pc, reg_name(rd, sizeof(word_t)), src1, src2, SEXT(src1 + src2, 32));
  );
  // 逻辑左移，左移位数为rs2[4:0], 对32位值操作，结果符号扩展成64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw, RR, 
    src2 &= 0b11111;
    uint32_src1 = (uint32_t)src1;
    uint32_src1 = uint32_src1 << src2;
    R(rd) = SEXT(uint32_src1, 32);
    // Log("sllw: %lx left shift %ld to %lx, save into %s", src1, src2, SEXT(uint32_src1, 32), reg_name(rd, sizeof(word_t)));
  );
  // 64位减法
  INSTPAT("0100000 ????? ????? 000 ????? 0110011", sub, RR, R(rd) = src1 - src2);
  // 32位减法
  INSTPAT("0100000 ????? ????? 000 ????? 0111011", subw, RR, 
      uint32_src1 = src1;
      uint32_src2 = src2;
      uint32_src1 = uint32_src1 - uint32_src2;
      R(rd) = SEXT(uint32_src1, 32);
  );
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 111 ????? 0110011", and, RR, R(rd) = src1 & src2);
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 110 ????? 0110011", and, RR, R(rd) = src1 | src2);
  // 有符号比较
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, RR, 
    int64_src1 = (int64_t)src1;
    int64_src2 = (int64_t)src2;
    R(rd) = (int64_src1 < int64_src2) ?  1 : 0;
    // if (int64_src1 < int64_src2) {
    //   Log("slt: src1: %ld < src2:%ld, R(rd)(%d) set to 1 ", int64_src1, int64_src2, rd);
    // } else{
    //   Log("slt: src1: %ld >= src2:%ld, R(rd)(%d) set to 0 ", int64_src1, int64_src2, rd);
    // }
  );

  // 符号扩展后比较
  //        imm[11:0]  | rs1 |   | rd  | opcode
  INSTPAT("???????????? ????? 010 ????? 0010011", slti, I, 
    R(rd) = (src1 < imm) ?  1 : 0;
    // if (src1 < imm) {
    //   Log("slti: src1: %lu < imm:%lu, %s set to 1 ", src1, imm, reg_name(rd, sizeof(word_t)));
    // } else{
    //   Log("slti: src1: %lu >= imm:%lu, %s set to 0 ", src1, imm, reg_name(rd, sizeof(word_t)));
    // }
  );

  // 无符号比较
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, RR, 
    R(rd) = (src1 < src2) ?  1 : 0;
    // if (src1 < src2) {
    //   Log("sltu: src1: %lu < src2:%lu, %s set to 1 ", src1, src2, reg_name(rd, sizeof(word_t)));
    // } else{
    //   Log("sltu: src1: %lu >= src2:%lu, %s set to 0 ", src1, src2, reg_name(rd, sizeof(word_t)));
    // }
  );
  // 有符号相乘
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000001 ????? ????? 000 ????? 0110011", mul, RR, 
    R(rd) = src1 * src2;
    // Log("mul: src1: 0x%lx * src2:0x%lx = 0x%lx, saving into %s", src1, src2, R(rd), reg_name(rd, sizeof(word_t)));
  );
  // 有符号低32位相除，结果的低32位有符号扩展至64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000001 ????? ????? 101 ????? 0110011", divu, RR, 
    R(rd) = src1 / src2;
    // Log("divu: src1: 0x%lx / src2:0x%lx = 0x%lx, saving into %s", src1, src2, R(rd), reg_name(rd, sizeof(word_t)));

  );
  // 有符号低32位相乘，结果的低32位有符号扩展至64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw, RR, 
    int32_src1 = src1;
    int32_src2 = src2;
    int32_src1 = int32_src1 * int32_src2;
    R(rd) = SEXT(int32_src1, 32);
    // Log("mulw: src1: 0x%lx * src2:0x%lx = 0x%lx, saving into %s", src1, src2, R(rd), reg_name(rd, sizeof(word_t)));

  );
  // 有符号低32位相除，结果的低32位有符号扩展至64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw, RR, 
    int32_src1 = src1;
    int32_src2 = src2;
    int32_src1 = int32_src1 / int32_src2;
    R(rd) = SEXT(int32_src1, 32);
    // Log("divw: src1: 0x%lx / src2:0x%lx = 0x%lx, saving into %s", src1, src2, R(rd), reg_name(rd, sizeof(word_t)));

  );
  // 低32位相除的余数，结果的低32位有符号扩展至64位
  //              | rs2 | rs1 |   |rd   | opcode
  INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw, RR, 
    int32_src1 = src1;
    int32_src2 = src2;
    int32_src1 = int32_src1 % int32_src2;
    R(rd) = SEXT(int32_src1, 32);
    // Log("remw: src1: 0x%lx %% src2:0x%lx = 0x%lx, saving into %s", src1, src2, R(rd), reg_name(rd, sizeof(word_t)));
  );

  //          imm     |  rs1 |   |rd   | opcode
  INSTPAT("???????????? ????? 011 ????? 00100 11", sltiu, I, R(rd) = (src1 < imm) ? 1 : 0);  // extand from seqz




  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
