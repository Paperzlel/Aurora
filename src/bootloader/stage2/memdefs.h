#pragma once

// 0x00000000 - 0x000003ff - Interrupt Vector Table
// 0x00000400 - 0x000004ff - Bios Data Area

#define MEMORY_STAGE_2_MIN 0x00000500       // Location of the bottom of stage 2 in memory
#define MEMORY_STAGE_2_MAX 0x00007bff       // Max location of stage 2 in memory (after here it overrides the boot sector)

#define MEMORY_BOOT_SECTOR_MIN 0x00007c00   // Min location of the boot sector
#define MEMORY_BOOT_SECTOR_MAX 0x00007dff   // Max location of the boot sector

#define MEMORY_AVAILABLE_MIN 0x00007e00
#define MEMORY_AVAILABLE_MAX 0x0007ffff

#define MEMORY_FAT_FS (void *)      0x00010000  // Pointer to FAT filesystem offset
#define MEMORY_FAT_SIZE             0x00010000  // Load at address, save space for rest of the filesystem

#define KERNEL_LOAD_ADDR (void *)   0x00020000  // Pointer to kernel load offset
#define KERNEL_LOAD_SIZE            0x00050000  // Kernel loads here, move on later in life

// NOTE: Kernel ELF may need to be moved elsewhere/loaded slowly as its max size as-is is ~320 KiB, which may not be enough in the future.

// Extended BIOS data area      0x00080000 - 0x0009ffff
// Video display memory area    0x000a0000 - 0x000bffff
// Video BIOS area              0x000c0000 - 0x000c7fff
// BIOS expansion area          0x000c8000 - 0x000effff
// Motherboard BIOS area        0x000f0000 - 0x000fffff

#define KERNEL_BASE_ADDR (void *)   0x00100000  // Actual location of the kernel
