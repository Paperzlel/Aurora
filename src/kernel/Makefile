TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding
TARGET_LDFLAGS += -T linker.ld -nostdlib
TARGET_CINCLUDES += -I../libs -Iinclude -I.
TARGET_LIBS += -lgcc -lk

SOURCES_C = $(subst ./,,$(shell find -type f -name "*.c"))
SOURCES_ASM = $(subst ./,,$(shell find -type f -name "*.asm"))
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))
DIRECTORIES = $(subst .,,$(subst ./,/,$(subst $(BUILD_DIR),,$(shell find -type d))))

.PHONY: all kernel clean scaffold

all: scaffold kernel

scaffold:
	@mkdir -p $(BUILD_DIR)/kernel/asm
	@mkdir -p $(BUILD_DIR)/kernel/c
	@mkdir -p $(addprefix $(BUILD_DIR)/kernel/asm,$(DIRECTORIES))
	@mkdir -p $(addprefix $(BUILD_DIR)/kernel/c,$(DIRECTORIES))

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(OBJECTS_ASM) $(OBJECTS_C)
	@$(TARGET_LD) $(TARGET_LDFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)
	@echo Created kernel.elf

$(BUILD_DIR)/kernel/c/%.o: %.c
	@echo 	Compiling $<...
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $< $(TARGET_CINCLUDES) $(TARGET_CDEFINES)


$(BUILD_DIR)/kernel/asm/%.o: %.asm
	@echo 	Compiling $<...
	@$(ASM) $(TARGET_ASMFLAGS) -o $@ $< 

install:
	@cp -R --preserve=timestamps include/. $(INCLUDEDIR)/.

clean:
	@rm -f $(BUILD_DIR)/kernel.elf

-include $(OBJECTS_C:.o=.d)