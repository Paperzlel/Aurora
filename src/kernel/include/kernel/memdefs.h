#pragma once

#define KERNEL_PHYSICAL_ADDRESS             0x00100000          // Physical address of the kernel in memory
#define KERNEL_VIRTUAL_ADDRESS     (void *) 0xc0100000          // Desired virtual address of the kernel data

#define PAGE_TABLE_VIRTUAL_ADDRESS (void *) 0xc0000000          // Virtual address of our page tables
#define PAGE_TABLE_MEMORY_SIZE              0x00100000          // Size of the page table in bytes

#define KERNEL_ALLOC_VIRTUAL_ADDRESS (void *) 0x40000000        // Start of the virtual address range
#define KERNEL_ALLOC_END_VIRTUAL_ADDRESS (void *)0xa0000000     // End of the virtual address range