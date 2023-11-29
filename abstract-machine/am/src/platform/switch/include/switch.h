#ifndef SWITCH_H__
#define SWITCH_H__

#include <klib-macros.h>
#include <riscv/riscv.h>

# define switch_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))

#define SERIAL_PORT     0x8020000
#define KBD_ADDR        0x8050000
#define RTC_ADDR        0x8030000
#define VGACTL_ADDR     0x0000000 // not supported yet
#define AUDIO_ADDR      0x0000000 // not supported yet
#define DISK_ADDR       0x0000000 // not supported yet
#define FB_ADDR         0x0000000 // not supported yet
#define AUDIO_SBUF_ADDR 0x0000000 // not supported yet
#define CMEM_ADDR       0x8040000
#define SW_ADDR         0x8060000
#define BTN_ADDR        0x8070000
#define LED_ADDR        0x8080000
#define SEG_ADDR        0x8090000

extern char _pmem_start;
#define PMEM_SIZE (256 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

#define PGSIZE    4096

#endif
