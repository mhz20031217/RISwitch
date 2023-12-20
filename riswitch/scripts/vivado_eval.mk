# Evaluation memory generation

include scripts/vivado.mk
.DEFAULT_GOAL = image

VIVADO_IMEM_COE = $(BUILD_DIR)/imem_img.coe
VIVADO_DMEM_COE = $(BUILD_DIR)/dmem_img.coe

ifeq ($(IMEM_COE),)
$(error Invalid IMEM_COE.)
endif

ifeq ($(DMEM_COE),)
$(error Invalid DMEM_COE.)
endif

image:
	cp $(IMEM_COE) $(VIVADO_IMEM_COE)
	cp $(DMEM_COE) $(VIVADO_DMEM_COE)
