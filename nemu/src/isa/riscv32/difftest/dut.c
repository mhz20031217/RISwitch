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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool flag = true;
  for(int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    if (cpu.gpr[i] != ref_r->gpr[i]) {
      flag = false;
      break;
    }
  }
  if (!flag) {
    Error("Difftest failed at pc: 0x%x", pc);
    Error("ID\t|D\t|R\tgprs:");
    for(int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
      Error("%d\t0x%x\t0x%x", i, cpu.gpr[i], ref_r->gpr[i]);
    }
  }
  return flag;
}

void isa_difftest_attach() {
}
