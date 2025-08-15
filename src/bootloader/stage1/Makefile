ASM?=nasm
BUILD_DIR?=build

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: boot.asm
	@echo	Compiling $<...
	@$(ASM) $< -f bin -o $@

clean:
	@rm -f $(BUILD_DIR)/stage1.bin