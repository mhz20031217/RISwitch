# scripts/nvdl_sim.mk
# Simulate on NVDL

include scripts/nvdl.mk

BIN = V$(SIM_TOP)_sim
WAVE = $(BUILD_DIR)/$(SIM_TOP).vcd
WAVECFG = $(TEST_DIR)/$(SIM_TOP).gtkw
SIM_TIMESTAMP = $(BUILD_DIR)/.sim.$(SIM_TOP).timestamp
.DEFAULT_GOAL = sim

VERILATOR_FLAGS += -DSIM --trace --top $(SIM_TOP) --public
CSRCS += $(shell find $(abspath $(TEST_DIR)) $(CSRC_PATTERN))
VSRCS += $(shell find $(abspath $(TEST_DIR)) $(VSRC_PATTERN))
HEADERS += $(shell find $(abspath $(TEST_DIR)) $(HEADER_PATTERN))

CFLAGS += $(addprefix -I,$(abspath $(INCLUDE_PATH)))

.PHONY: wave
wave: $(WAVE)
	@echo "### WAVEFORM ###"
	@$(GTKWAVE) --save=$(WAVECFG) --saveonexit $(WAVE)

$(WAVE): $(SIM_TIMESTAMP)

.PHONY: sim
sim: $(SIM_TIMESTAMP)

$(SIM_TIMESTAMP): $(BUILD_DIR)/$(BIN)
	$(call git_commit, "sim RTL")
	@echo "### SIMULATION ###"
	@cd $(BUILD_DIR) && ./$(BIN)
	@touch $(SIM_TIMESTAMP)

$(BUILD_DIR)/$(BIN): $(CSRCS) $(VSRCS) $(HEADERS)
	@echo "### COMPILATION ###"
	$(VERILATOR) $(VERILATOR_FLAGS) \
		$(addprefix -CFLAGS , $(CFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(VSRCS) $(CSRCS) -o $(BIN)

.PHONY: compile
compile: $(BUILD_DIR)/$(BIN)

