include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/switch.mk
COMMON_CFLAGS += -march=rv32i_zicsr -mabi=ilp32  # overwrite
LDFLAGS       += -melf32lriscv                    # overwrite

AM_SRCS += riscv/switch/libgcc/div.S \
           riscv/switch/libgcc/muldi3.S \
           riscv/switch/libgcc/multi3.c \
           riscv/switch/libgcc/ashldi3.c \
           riscv/switch/libgcc/unused.c
