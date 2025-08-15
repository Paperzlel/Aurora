TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding
TARGET_LDFLAGS += -T linker.ld -nostdlib
TARGET_CINCLUDES += -I../libs
TARGET_LIBS += -lgcc

SOURCES_C = $(wildcard *.c)
SOURCES_ASM = $(wildcard *.asm)
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))

.PHONY: all kernel clean scaffold

all: scaffold kernel

scaffold:
	@mkdir -p $(BUILD_DIR)/kernel/asm
	@mkdir -p $(BUILD_DIR)/kernel/c

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo Created kernel.bin

$(BUILD_DIR)/kernel/c/%.o: %.c
	@echo 	Compiling $<...
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $< $(TARGET_CINCLUDES)


$(BUILD_DIR)/kernel/asm/%.o: %.asm
	@echo 	Compiling $<...
	@$(ASM) $(TARGET_ASMFLAGS) -o $@ $< 

clean:
	@rm -f $(BUILD_DIR)/kernel.bin