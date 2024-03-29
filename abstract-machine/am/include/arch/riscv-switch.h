#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

#define SWITCH_EXIT_SUCCESS 0x00c0ffee
#define SWITCH_EXIT_FAIL 0xdeadbeef

struct Context {
  uintptr_t gpr[NR_REGS];
  uintptr_t mcause, mstatus, mepc;

  void *pdir;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
