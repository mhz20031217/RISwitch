BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))

CSRCS = gen.c
ASM_SRCS = csr.S
ELF = $(BUILD_DIR)/csr.elf
IMG = $(BUILD_DIR)/csr.bin
GEN = $(BUILD_DIR)/gen

CFLAGS = -march=rv32i_zicsr

clean:
	rm -rf $(BUILD_DIR)

$(ELF): $(ASM_SRCS)
	riscv32-unknown-elf-gcc $(CFLAGS) -c -o $(ELF) $(ASM_SRCS)

$(IMG): $(ELF)
	riscv32-unknown-elf-objcopy --dump-section .text=$(IMG) $(ELF)

$(GEN): $(CSRCS)
	gcc -o $(GEN) $(CSRCS)
	cd $(BUILD_DIR) && ./gen

all: $(GEN)
.DEFAULT_GOAL = all

.PHONY: all clean
