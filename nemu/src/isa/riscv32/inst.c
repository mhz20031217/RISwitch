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

#include "common.h"
#include "isa.h"
#include "local-include/reg.h"
#include "macro.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define R(i) gpr(i)
#define C(i) csr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_R, TYPE_I, TYPE_S, TYPE_B, TYPE_U, TYPE_J,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immB() do { *imm = \
  (SEXT(BITS(i, 31, 31), 1) << 12) \
| (BITS(i, 7, 7) << 11) \
| (BITS(i, 30, 25) << 5) \
| (BITS(i, 11, 8) << 1); \
} while(0)
#define immJ() do { *imm = \
  (SEXT(BITS(i, 31, 31), 1) << 20) \
| (BITS(i, 19, 12) << 12) \
| (BITS(i, 20, 20) << 11) \
| (BITS(i, 30, 21) << 1); \
} while(0)

static inline word_t setbit(word_t x, int idx) {
  return x | (1U << idx);
}

static inline word_t rstbit(word_t x, int idx) {
  return x & ~(1U << idx);
}

static inline word_t getbit(word_t x, int idx) {
  return x & (1U << idx);
}

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_J:                   immJ(); break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
   // Decode
  uint32_t i = s->isa.inst.val;
  word_t csr = BITS(i, 31, 20), zimm = BITS(i, 19, 15);
   s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

#define SCMP(a, b) ((*((sword_t*)&(a)))<(*((sword_t*)&(b)))?1:0)
#define UCMP(a, b) (((a)<(b))?1:0)
#define BCOND(cond) if ((cond)) s->dnpc = s->pc + imm

  INSTPAT_START();

  /* arith */
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2);
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);
  // @Reference: Spike https://github.com/riscv-software-src/riscv-isa-sim
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, 
    R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, 
    R(rd) = SEXT((SEXT(src1, 32)) * SEXT(src2, 32) >> 32, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, 
    R(rd) = SEXT(((uint64_t)(uint32_t)src1 * (uint64_t)(uint32_t)src2) >> 32, 32));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, 
    R(rd) = SEXT((SEXT(src1, 32) * ((uint64_t)(uint32_t)src2)) >> 32, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, 
    sword_t l = src1;
    sword_t r = src2;
    if (r == 0) R(rd) = ~0U;
    else if (l == INT32_MIN && r == -1) R(rd) = l;
    else R(rd) = l / r;
  );
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, 
    uint64_t l = src1;
    uint64_t r = src2;
    if (r == 0) R(rd) = ~0U;
    else R(rd) = SEXT(l / r, 64);
  );
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, 
    sword_t l = src1;
    sword_t r = src2;
    if (r == 0) R(rd) = l;
    else if (l == INT32_MIN && r == -1) R(rd) = 0;
    else R(rd) = l % r;
  );
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, 
    uint64_t l = src1;
    uint64_t r = src2;
    if (r == 0) R(rd) = SEXT(l, 64);
    else R(rd) = SEXT(l % r, 64);
  );

  /* bit operation */
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << BITS(src2, 4, 0));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << BITS(imm, 4, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (*(sword_t*)&src1) >> BITS(src2, 4, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = (*(sword_t*)&src1) >> BITS(imm, 4, 0));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = BITS(src1, 31, BITS(src2, 4, 0)));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = BITS(src1, 31, BITS(imm, 4, 0)));

  /* compare */
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = SCMP(src1, src2));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = UCMP(src1, src2));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = SCMP(src1, imm));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = UCMP(src1, imm));

  /* branch */
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, BCOND(src1 == src2));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, BCOND(src1 != src2));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, BCOND(SCMP(src1, src2)));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, BCOND(UCMP(src1, src2)));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, BCOND(!SCMP(src1, src2)));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, BCOND(!UCMP(src1, src2)));


  /* load */
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  
  // imms of U-type have been shifted
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  
  /* store */
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));

  /* jump */
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  #ifdef CONFIG_FTRACE
  void ftrace_call(uint32_t pc, uint32_t addr);
  void ftrace_ret(uint32_t pc, uint32_t addr);
  #endif
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, 
    R(rd) = s->pc + 4;
    s->dnpc = s->pc + imm;
    #ifdef CONFIG_FTRACE
    if (FTRACE_COND) {
      if (BITS(s->isa.inst.val, 11, 7) == 1) {
        // fprintf(stderr, "jal called and rd is x1.\n");
        ftrace_call(s->pc, s->dnpc);
      }
    }
    #endif
  );
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I,
    s->dnpc = (src1 + imm)&~1;
    R(rd) = s->pc + 4;
    #ifdef CONFIG_FTRACE
    if (FTRACE_COND) {
      if (BITS(s->isa.inst.val, 19, 15) == 1) {
        // fprintf(stderr, "jalr called and rs1 is x1.\n");
        ftrace_ret(s->pc, s->dnpc);
      } else if (BITS(s->isa.inst.val, 11, 7) == 1) {
        // fprintf(stderr, "jalr called and rd is x1.\n");
        ftrace_call(s->pc, s->dnpc);
      }
    }
    #endif
  );

  /* Ziscr */
  INSTPAT("???????????? ????? 011 ????? 11100 11",  csrrc  , I,
    word_t t = C(csr);
    C(csr) = t & ~src1;
    R(rd) = t;
  );
  INSTPAT("???????????? ????? 111 ????? 11100 11", csrrci  , I,
    word_t t = C(csr);
    C(csr) = t & ~zimm;
    R(rd) = t;
  );
  INSTPAT("???????????? ????? 010 ????? 11100 11",  csrrs  , I,
    word_t t = C(csr);
    C(csr) = t | ~src1;
    R(rd) = t;
  );
  INSTPAT("???????????? ????? 110 ????? 11100 11", csrrsi  , I,
    word_t t = C(csr);
    C(csr) = t | ~zimm;
    R(rd) = t;
  );
  INSTPAT("???????????? ????? 001 ????? 11100 11",  csrrw  , I,
    word_t t = C(csr);
    C(csr) = src1;
    R(rd) = t;
  );
  INSTPAT("???????????? ????? 101 ????? 11100 11", csrrwi  , I,
    R(rd) = C(csr);
    C(csr) = zimm;
  );
  INSTPAT("000000000000 00000 000 00000 11100 11", ecall   , I,
    isa_raise_intr(11, s->pc);
    s->dnpc = C(CSR_MTVEC_IDX);
  );
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , R,
    s->dnpc = C(CSR_MEPC_IDX);
    word_t mstatus = C(CSR_MSTATUS_IDX);
    word_t mpie = getbit(mstatus, 7);
    mstatus = (mpie) ? setbit(mstatus, 3) : rstbit(mstatus, 3);
  );

  /* nemu */
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();


  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
