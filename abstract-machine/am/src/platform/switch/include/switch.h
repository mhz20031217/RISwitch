#ifndef SWITCH_H__
#define SWITCH_H__

#include <klib-macros.h>
#include <riscv/riscv.h>

#define switch_trap(code) \
    asm volatile( \
        "mv a0, %0;\r\n" \
        ".word 0xdead10cc\r\n" : :"r"(code))

#define SERIAL_PORT     0x80f00000
#define KBD_ADDR        0x80500000
#define RTC_ADDR        0x80900000
#define VGACTL_ADDR     0x80e00000
#define AUDIO_ADDR      0x00000000 // not supported yet
#define DISK_ADDR       0x00000000 // not supported yet
#define FB_ADDR         0x80800000
#define AUDIO_SBUF_ADDR 0x00000000 // not supported yet
#define CMEM_ADDR       0x80400000
#define SW_ADDR         0x80600000
#define LED_ADDR        0x80700000
#define SEG_ADDR        0x80200000

extern char _pmem_start;
#define PMEM_SIZE (256 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

#define PGSIZE    4096

#endif
