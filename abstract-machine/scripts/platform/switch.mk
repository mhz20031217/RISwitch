AM_SRCS := riscv/switch/start.S \
           riscv/switch/trm.c \
           riscv/switch/ioe.c \
           riscv/switch/timer.c \
           riscv/switch/input.c \
           riscv/switch/cte.c \
           riscv/switch/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

# Overwrite ARCH_H
ARCH_H := arch/riscv-switch.h

CFLAGS    += -fdata-sections -ffunction-sections -Os
LDFLAGS   += -T $(AM_HOME)/scripts/platform/switch.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
LDFLAGS   += --print-memory-usage
CFLAGS += -DMAINARGS=\"$(mainargs)\"
CFLAGS += -I$(AM_HOME)/am/src/platform/switch/include
.PHONY: $(AM_HOME)/am/src/riscv/switch/trm.c

IMEM_IMG = $(IMAGE_REL).hex
DMEM_IMG = $(IMAGE_REL)_d.hex
IMEM_COE = $(IMAGE_REL).coe
DMEM_COE = $(IMAGE_REL)_d.coe

PYTHON = python3
HEXGEN = $(AM_HOME)/tools/switch-img.py

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --only-section=.text* -O binary $(IMAGE).elf $(IMAGE).bin
	@echo + OBJCOPY "->" $(IMAGE_REL)_d.bin
	@$(OBJCOPY) -S --only-section=*data* --only-section=*bss* --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE)_d.bin
	@echo + HEXGEN "->" $(IMAGE_REL).hex \(InstrMem Image\)
	@$(PYTHON) $(HEXGEN) $(IMAGE_REL).bin $(IMEM_IMG) $(IMEM_COE)
	@echo + HEXGEN "->" $(IMAGE_REL)_d.hex \(DataMem Image\)
	@$(PYTHON) $(HEXGEN) $(IMAGE_REL)_d.bin $(DMEM_IMG) $(DMEM_COE)
	
sim: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(abspath $(IMEM_IMG))" DMEM_IMG="$(abspath $(DMEM_IMG))" PLATFORM=NVDL MODE=SIM

wave: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(abspath $(IMEM_IMG))" DMEM_IMG="$(abspath $(DMEM_IMG))" PLATFORM=NVDL MODE=SIM wave

eval: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(abspath $(IMEM_IMG))" DMEM_IMG="$(abspath $(DMEM_IMG))" PLATFORM=NVDL MODE=EVAL nvboard

evalwave: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(abspath $(IMEM_IMG))" DMEM_IMG="$(abspath $(DMEM_IMG))" PLATFORM=NVDL MODE=EVAL wave

cleansim:
	$(MAKE) -C $(SWITCH_HOME) clean

vivado: image
	$(MAKE) -C $(SWITCH_HOME) IMEM_IMG="$(abspath $(IMEM_IMG))" DMEM_IMG="$(abspath $(DMEM_IMG))" IMEM_COE="$(abspath $(IMEM_COE))" DMEM_COE="$(abspath $(DMEM_COE))" PLATFORM=VIVADO MODE=EVAL
