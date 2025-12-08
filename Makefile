export ASM=nasm
export CC=gcc
export CXX=g++
export LD=gcc
export CFLAGS=
export LINKFLAGS=
export LIBS=


export SRC_DIR=src
export BUILD_DIR=build
export TOOLS_DIR=toolchain
export SYSROOT=$(abspath $(BUILD_DIR))/sysroot
export USR_INCLUDE=/usr/include
export USR_LIB=/usr/lib
export INCLUDEDIR=$(SYSROOT)$(USR_INCLUDE)
export LIBDIR=$(SYSROOT)$(USR_LIB)

export TARGET=i686-aurora
export TARGET_ARCH=i686				# Use -march=[this] to load a specific architecture, add to both GCC and LD
export TARGET_CC=$(TARGET)-gcc --sysroot=$(SYSROOT) -isystem=$(USR_INCLUDE)
export TARGET_CXX=$(TARGET)-g++
export TARGET_LD=$(TARGET)-gcc --sysroot=$(SYSROOT) -isystem=$(USR_INCLUDE)
export TARGET_OBJCOPY=$(TARGET)-objcopy
export TARGET_AR=$(TARGET)-ar
export TARGET_ASMFLAGS= 
export TARGET_CFLAGS=-std=c99 -g -MD -Wall -Wextra -Wno-sign-compare
export TARGET_CINCLUDES=
export TARGET_CDEFINES=-D__I386__ -D__x86__
export TARGET_LDFLAGS= 
export TARGET_LIBS=

.PHONY: all scaffold install bootloader kernel floppy_image clean toolchain libc libk

all: scaffold install bootloader libk kernel floppy_image

#
# Building the OS itself
#

scaffold:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(INCLUDEDIR)
	@mkdir -p $(LIBDIR)

# Floppy image

floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader libk kernel
	@dd if=/dev/zero of=$@ bs=512 count=2880 2> /dev/null
	@mkfs.fat -F 12 -n "AUOS" $@ 2> /dev/null
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ conv=notrunc 2> /dev/null
	@mcopy -i $@ $(BUILD_DIR)/stage2.bin "::stage2.bin" 2> /dev/null
	@mcopy -i $@ $(BUILD_DIR)/kernel.elf "::kernel.elf" 2> /dev/null
	@mmd -i $@ "::dev" 2> /dev/null
	@mcopy -i $@ $(PWD)/resources/test.txt "::dev/test.txt" 2> /dev/null
	@mcopy -i $@ $(PWD)/resources/Lat2-Fixed16.psf "::dev/font.psf" 2> /dev/null
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

libc:

libk: $(BUILD_DIR)/libk.a

$(BUILD_DIR)/libk.a: scaffold
	@$(MAKE) -C src/libc BUILD_DIR=$(abspath $(BUILD_DIR))

install:
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) install
	@$(MAKE) -C src/libc BUILD_DIR=$(abspath $(BUILD_DIR)) install

clean:
	@rm -rf $(BUILD_DIR)/*

#
# Building the toolchain (GCC cross-compilation)
#

PREFIX=$(abspath $(TOOLS_DIR)/$(TARGET))

export PATH := $(PREFIX)/bin:$(PATH)

toolchain: toolchain_binutils toolchain_gcc

BINUTILS_VERSION=2.37
BINUTILS_URL=https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz
BINUTILS_SRC=$(TOOLS_DIR)/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD=$(TOOLS_DIR)/binutils-build-$(BINUTILS_VERSION)
BINUTILS_DIFF_FILE=binutils.diff


toolchain_binutils: $(PREFIX)/bin/$(TARGET)-ld

$(PREFIX)/bin/$(TARGET)-ld: $(BINUTILS_SRC)
	./scripts/apply_diff.sh binutils $(BINUTILS_VERSION)
	@cp scripts/elf_i386_aurora.sh $(TOOLS_DIR)/binutils-$(BINUTILS_VERSION)/ld/emulparams/elf_i386_aurora.sh
	@cp scripts/elf_x86_64_aurora.sh $(TOOLS_DIR)/binutils-$(BINUTILS_VERSION)/ld/emulparams/elf_x86_64_aurora.sh
	@mkdir -p $(BINUTILS_BUILD)
	@cd $(BINUTILS_BUILD) && ../binutils-$(BINUTILS_VERSION)/configure \
			--prefix="$(PREFIX)"			\
			--target=$(TARGET)				\
			--with-sysroot=$(SYSROOT)		\
			--disable-nls					\
			--disable-werror
	@$(MAKE) -j8 -C $(BINUTILS_BUILD)
	@$(MAKE) -C $(BINUTILS_BUILD) install


$(BINUTILS_SRC):
	@mkdir -p $(TOOLS_DIR)
	@cd $(TOOLS_DIR) && wget $(BINUTILS_URL)
	@tar -xf binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION=11.2.0
GCC_URL=https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz
GCC_SRC=$(TOOLS_DIR)/gcc-$(GCC_VERSION)
GCC_BUILD=$(TOOLS_DIR)/gcc-build-$(GCC_VERSION)
GCC_DIFF_FILE=gcc.diff


toolchain_gcc: $(PREFIX)/bin/$(TARGET)-gcc

$(PREFIX)/bin/$(TARGET)-gcc: $(PREFIX)/bin/$(TARGET)-ld $(GCC_SRC)
	./scripts/apply_diff.sh gcc $(GCC_VERSION)
	cp scripts/aurora.h $(TOOLS_DIR)/gcc-$(GCC_VERSION)/gcc/config/aurora.h
	@mkdir -p $(GCC_BUILD)
	@cd $(GCC_BUILD) && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../gcc-$(GCC_VERSION)/configure \
			--prefix="$(PREFIX)"		\
			--target=$(TARGET)			\
			--disable-nls				\
			--enable-languages=c,c++	\
			--with-sysroot=$(SYSROOT)
	@$(MAKE) -j8 -C $(GCC_BUILD) all-gcc all-target-libgcc
	@$(MAKE) -C $(GCC_BUILD) install-gcc install-target-libgcc

$(GCC_SRC):
	@mkdir -p $(TOOLS_DIR)
	@cd $(TOOLS_DIR) && wget $(GCC_URL)
	@tar -xf gcc-$(GCC_VERSION).tar.xz

clean_toolchain:
	@rm -rf $(GCC_BUILD) $(GCC_SRC) $(BINUTILS_BUILD) $(BINUTILS_SRC)

clean_toolchain_all:
	@rm -rf $(TOOLS_DIR)/*
