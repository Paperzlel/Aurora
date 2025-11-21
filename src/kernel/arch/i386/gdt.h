#pragma once

#define KERNEL32_CODE_SEGMENT 0x08   // Value for cs to point to
#define KERNEL32_DATA_SEGMENT 0x10   // Value for ds to point to

/**
 * @brief Initializes the kernel's General Descriptor Table, which is functionally the same as the bootloader's, but needs to be re-loaded due to our
 * trips back and forth from Real Mode.
 */
void i386_gdt_initialize();