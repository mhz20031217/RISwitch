# scripts/nvdl_eval.mk
# Evaluate on NVDL using nvboard

include scripts/nvdl.mk
.DEFAULT_GOAL=nvboard

BIN = V$(EVAL_TOP)_eval
WAVE = $(BUILD_DIR)/$(EVAL_TOP).vcd
WAVECFG = $(TOP_DIR)/$(EVAL_TOP).gtkw
include $(NVBOARD_HOME)/scripts/nvboard.mk
# Add NVBoard INC_PATH to INCLUDE_PATH
INCLUDE_PATH += $(INC_PATH)
INCLUDE_PATH += $(NVBOARD_HOME)/include

VERILATOR_FLAGS += --top $(EVAL_TOP) -DEVAL --trace
CSRCS += $(shell find $(abspath $(TOP_DIR)) $(CSRC_PATTERN))
VSRCS += $(shell find $(abspath $(TOP_DIR)) $(VSRC_PATTERN))

CFLAGS += $(addprefix -I,$(abspath $(INCLUDE_PATH))) -DTOP_NAME="\"V$(EVAL_TOP)\""L $(shell sdl2-config --cflags)
LDFLAGS += $(NVBOARD_ARCHIVE) -lSDL2 -lSDL2_image

# Clock type
ifeq ($(CLOCK_TYPE),RT)
	CFLAGS += -DCLK_RT
	VERILATOR_FLAGS += -DCLK_RT
else
	CFLAGS += -DCLK_PERF
	VERILATOR_FLAGS += -DCLK_PERF
endif



SRC_AUTOBIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
CSRCS += $(SRC_AUTOBIND)
$(SRC_AUTOBIND): $(NXDC_FILE)
	$(PYTHON) $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

.PHONY: nvboard
nvboard: compile
	$(call git_commit, "eval RTL")
	@echo "### EVALUATION ###"
	@cd $(BUILD_DIR) && ./$(BIN) $(IMEM_IMG) $(DMEM_IMG)

wave:
	@echo "### WAVEFORM ###"
	@$(GTKWAVE) --save=$(WAVECFG) --saveonexit $(WAVE)

$(BUILD_DIR)/$(BIN): $(VSRCS) $(CSRCS) nvboard-archive
	@echo "VSRCS: " $(VSRCS)
	@echo "CSRCS: " $(CSRCS)
	$(VERILATOR) $(VERILATOR_FLAGS) \
	$(addprefix -CFLAGS , $(CFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
	$(VSRCS) $(CSRCS) -o $(BIN)

.PHONY: compile
compile: $(BUILD_DIR)/$(BIN)
	@echo "### COMPILATION ###"
