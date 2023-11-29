# scripts/nvdl_sim.mk
# Simulate on NVDL

include scripts/nvdl.mk

BIN = V$(SIM_TOP)_sim
WAVE = $(BUILD_DIR)/$(SIM_TOP).vcd
.DEFAULT_GOAL = sim

VERILATOR_FLAGS += -DSIM --trace --top $(SIM_TOP) --public
CSRCS += $(shell find $(abspath $(TEST_DIR)) $(CSRC_PATTERN))
VSRCS += $(shell find $(abspath $(TEST_DIR)) $(VSRC_PATTERN))

CFLAGS += $(addprefix -I,$(abspath $(INCLUDE_PATH)))

.PHONY: wave
wave: $(WAVE)
	@echo "### WAVEFORM ###"
	@$(GTKWAVE) $(WAVE)

$(WAVE):sim

.PHONY: sim
sim: compile
	$(call git_commit, "sim RTL")
	@echo "### SIMULATION ###"
	@cd $(BUILD_DIR) && ./$(BIN)

$(BUILD_DIR)/$(BIN): $(CSRCS) $(VSRCS)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		$(addprefix -CFLAGS , $(CFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		$(VSRCS) $(CSRCS) -o $(BIN)

.PHONY: compile
compile: $(BUILD_DIR)/$(BIN)
	@echo "### COMPILATION ###"

