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

#include "debug.h"
#include <isa.h>

#define C(i) cpu.csr[i]

static inline word_t setbit(word_t x, int idx) {
  return x | (1U << idx);
}

static inline word_t rstbit(word_t x, int idx) {
  return x & ~(1U << idx);
}

static inline word_t getbit(word_t x, int idx) {
  return x & (1U << idx);
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  #ifdef CONFIG_ETRACE
  if (ETRACE_COND)
    Info("[etrace] Exception No.%d at epc: 0x%x", NO, epc);
  #endif
  
  C(CSR_MEPC_IDX) = epc;
  C(CSR_MCAUSE_IDX) = NO; // use Exception-from-M-mode
  word_t mstatus = C(CSR_MSTATUS_IDX);
  word_t mie = getbit(mstatus, 3);
  mstatus = (mie) ? setbit(mstatus, 7) : rstbit(mstatus, 0);
  mstatus = rstbit(mstatus, 3);
  C(CSR_MSTATUS_IDX) = mstatus;
  
  return 0;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
