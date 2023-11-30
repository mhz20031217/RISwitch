AM_SRCS := riscv/switch/start.S \
           riscv/switch/trm.c \
           riscv/switch/ioe.c \
           riscv/switch/timer.c \
           riscv/switch/input.c \
           riscv/switch/cte.c \
           riscv/switch/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/platform/switch.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
CFLAGS += -I$(AM_HOME)/am/src/platform/switch/include
.PHONY: $(AM_HOME)/am/src/riscv/switch/trm.c

IMEM_IMG = $(IMAGE).hex
DMEM_IMG = $(IMAGE)_d.hex

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(IMEM_IMG)" DMEM_IMG="$(DMEM_IMG)" PLATFORM=NVDL MODE=EVAL
