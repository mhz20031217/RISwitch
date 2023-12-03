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
#include <stdbool.h>
#include <string.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *csrs[] = {
  [SSTATUS] = "sstatus", [SIE] = "sie", [STVEC] = "stvec", [SSCRATCH] = "sscratch",
  [SEPC] = "sepc", [SCAUSE] = "scause", [STVAL] = "stval", [SIP] = "sip",
  [SATP] = "satp",
  [MSTATUS] = "mstatus", [MIE] = "mie", [MTVEC] = "mtvec", [MSCRATCH] = "mscratch",
  [MEPC] = "mepc", [MCAUSE] = "mcause", [MTVAL] = "mtval", [MIP] = "mip",
  [NR_CSR] = NULL
};

void isa_csr_display() {
  for (int i = 0; i < 4096; i ++) {
    if (csrs[i]) {
      printf("%10s "FMT_WORD"\t%16d\n", csr_name(i), csr(i), csr(i));
    }
  }
}

void isa_reg_display() {
  for(int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    printf("%10s "FMT_WORD"\t%16d\n", reg_name(i), gpr(i), gpr(i));
  }
  isa_csr_display();
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, "pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    if (strcmp(s, regs[i]) == 0) {
      *success = true;
      return cpu.gpr[i];
    }
  }
  *success = false;
  return 0;
}

word_t isa_csr_str2val(const char *s, bool *success) {
  for (int i = 0 ; i < 4096; i ++) {
    if (csrs[i] == NULL) continue;
    if (strcmp(s, csrs[i]) == 0) {
      *success = true;
      return cpu.csr[i];
    }
  }
  *success = false;
  return 0;
}
