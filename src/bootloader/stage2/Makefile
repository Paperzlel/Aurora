TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding
TARGET_LDFLAGS += -T linker.ld -nostdlib
TARGET_CINCLUDES += -I../../kernel/include
TARGET_LIBS += -lgcc

SOURCES_C = $(wildcard *.c)
SOURCES_ASM = $(wildcard *.asm)
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/stage2/c/%.o, $(SOURCES_C))
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.o, $(SOURCES_ASM))

.PHONY: all stage2 clean scaffold

all: scaffold stage2

scaffold:
	@mkdir -p $(BUILD_DIR)/stage2/asm
	@mkdir -p $(BUILD_DIR)/stage2/c

stage2: $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/stage2_debug.elf

$(BUILD_DIR)/stage2_debug.elf: $(BUILD_DIR)/stage2.bin
	@$(TARGET_LD) -T linker_debug.ld -nostdlib -o $@ $(OBJECTS_ASM) $(OBJECTS_C) $(TARGET_LIBS)
	@echo Created stage2_debug.elf

$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/stage2.map -o $@ $^ $(TARGET_LIBS)
	@echo Created stage2.bin

$(BUILD_DIR)/stage2/c/%.o: %.c
	@echo 	Compiling $<...
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $< $(TARGET_CINCLUDES) $(TARGET_CDEFINES)


$(BUILD_DIR)/stage2/asm/%.o: %.asm
	@echo 	Compiling $<...
	@$(ASM) $(TARGET_ASMFLAGS) -o $@ $< 

clean:
	@rm -f $(BUILD_DIR)/stage2.bin

-include $(OBJECTS_C:.o=.d)