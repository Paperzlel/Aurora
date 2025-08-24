export ASM=nasm
export CC=gcc
export CXX=g++
export LD=gcc
export CFLAGS=std=c99 -g -mtune=i386
export LINKFLAGS=
export LIBS=


export SRC_DIR=src
export BUILD_DIR=build
export TOOLS_DIR=toolchain

export TARGET=i686-elf
export TARGET_ARCH=i686				# Use -march=[this] to load a specific architecture, add to both GCC and LD
export TARGET_CC=$(TARGET)-gcc
export TARGET_CXX=$(TARGET)-g++
export TARGET_LD=$(TARGET)-gcc
export TARGET_ASMFLAGS= 
export TARGET_CFLAGS=-std=c99 -g
export TARGET_CINCLUDES=
export TARGET_LDFLAGS= 
export TARGET_LIBS=

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
	@dd if=/dev/zero of=$@ bs=512 count=2880 >/dev/null
	@mkfs.fat -F 12 -n "AUOS" $@ >/dev/null
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc >/dev/null
	@mcopy -i $@ $(BUILD_DIR)/stage2.bin "::stage2.bin"
	@mcopy -i $@ $(BUILD_DIR)/kernel.elf "::kernel.elf"
	@mmd -i $@ "::dev"
	@mcopy -i $@ test.txt "::dev/test.txt"
	@mcopy -i $@ NOTES.md "::dev/NOTES.md"
	@echo Created $@

# Bootloader

bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: scaffold
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: scaffold
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))


# Kernel

kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: scaffold stage1 stage2
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

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
