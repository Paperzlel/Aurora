ASM=nasm
CC=gcc
CXX=g++
LD=gcc
CFLAGS=std=c99 -g
LINKFLAGS=
LIBS=


SRC_DIR=src
BUILD_DIR=build
TOOLS_DIR=toolchain

TARGET=i686-elf
TARGET_CC=$(TARGET)-gcc
TARGET_CXX=$(TARGET)-g++
TARGET_LD=$(TARGET)-gcc

all: scaffold bootloader kernel floppy_image

.PHONY: all scaffold bootloader kernel floppy_image clean toolchain

#
# Building the OS itself
#

scaffold:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/kernel/c
	@mkdir -p $(BUILD_DIR)/kernel/asm

# Floppy image

floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880
	@mkfs.fat -F 12 -n "AUOS" $(BUILD_DIR)/main_floppy.img
	@dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc
	@mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

# Bootloader

bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: $(SRC_DIR)/bootloader/main.asm
	@$(ASM) $(SRC_DIR)/bootloader/main.asm -f bin -o $(BUILD_DIR)/bootloader.bin

# Kernel

SOURCES_KERNEL_C = $(wildcard src/kernel/*.c)
SOURCES_KERNEL_ASM = $(wildcard src/kernel/*.asm)
OBJECTS_KERNEL_C = $(patsubst src/kernel/%.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_KERNEL_C))
OBJECTS_KERNEL_ASM = $(patsubst src/kernel/%.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_KERNEL_ASM))

TARGET_CFLAGS=-std=c99 -g -m16 # HACK: Disable this when you can, it's needed to properly generate 16-bit assembly and doesn't work otherwise

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_KERNEL_ASM) $(OBJECTS_KERNEL_C)
	@$(TARGET_LD) $^ -T link.ld -nostdlib -o $@ -Wl,-Map=$(BUILD_DIR)/kernel.map -lgcc
# Use Open Watcom, not GCC. It's not wanting to play for raw binaries

$(BUILD_DIR)/kernel/c/%.o: $(SRC_DIR)/kernel/%.c
	@$(TARGET_CC) $(TARGET_CFLAGS) -ffreestanding -c -o $@ $<


$(BUILD_DIR)/kernel/asm/%.o: $(SRC_DIR)/kernel/%.asm
	@$(ASM) -f elf -o $@ $<

clean:
	@rm -rf $(BUILD_DIR)/*

#
# Building the toolchain (GCC cross-compilation)
#

PREFIX=$(abspath $(TOOLS_DIR)/$(TARGET))

export PATH := $(PREFIX)/bin:$(PATH)

BINUTILS_VERSION=2.37
BINUTILS_URL=https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz
BINUTILS_SRC=$(TOOLS_DIR)/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD=$(TOOLS_DIR)/binutils-build-$(BINUTILS_VERSION)

toolchain: toolchain_binutils toolchain_gcc


toolchain_binutils: $(PREFIX)/bin/$(TARGET)-ld

$(PREFIX)/bin/$(TARGET)-ld: $(BINUTILS_SRC).tar.xz
	@cd $(TOOLS_DIR) && tar -xf binutils-$(BINUTILS_VERSION).tar.xz
	@mkdir -p $(BINUTILS_BUILD)
	@cd $(BINUTILS_BUILD) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../binutils-$(BINUTILS_VERSION)/configure \
			--prefix="$(PREFIX)"			\
			--target=$(TARGET)				\
			--with-sysroot					\
			--disable-nls					\
			--disable-werror
	@$(MAKE) -j8 -C $(BINUTILS_BUILD)
	@$(MAKE) -C $(BINUTILS_BUILD) install


$(BINUTILS_SRC).tar.xz:
	@mkdir -p $(TOOLS_DIR)
	@cd $(TOOLS_DIR) && wget $(BINUTILS_URL)

GCC_VERSION=11.2.0
GCC_URL=https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz
GCC_SRC=$(TOOLS_DIR)/gcc-$(GCC_VERSION)
GCC_BUILD=$(TOOLS_DIR)/gcc-build-$(GCC_VERSION)


toolchain_gcc: $(PREFIX)/bin/$(TARGET)-gcc

$(PREFIX)/bin/$(TARGET)-gcc: $(PREFIX)/bin/$(TARGET)-ld $(GCC_SRC).tar.xz
	@cd $(TOOLS_DIR) && tar -xf gcc-$(GCC_VERSION).tar.xz
	@mkdir -p $(GCC_BUILD)
	@cd $(GCC_BUILD) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../gcc-$(GCC_VERSION)/configure \
			--prefix="$(PREFIX)"		\
			--target=$(TARGET)			\
			--disable-nls				\
			--enable-languages=c,c++	\
			--without-headers
	@$(MAKE) -j8 -C $(GCC_BUILD) all-gcc all-target-libgcc
	@$(MAKE) -C $(GCC_BUILD) install-gcc install-target-libgcc

$(GCC_SRC).tar.xz:
	@mkdir -p $(TOOLS_DIR)
	@cd $(TOOLS_DIR) && wget $(GCC_URL)

clean_toolchain:
	rm -rf $(GCC_BUILD) $(GCC_SRC) $(BINUTILS_BUILD) $(BINUTILS_SRC)

clean_toolchain_all:
	rm -rf $(TOOLS_DIR)/*
