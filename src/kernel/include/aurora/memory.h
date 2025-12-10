#ifndef _AURORA_MEMORY_H
#define _AURORA_MEMORY_H

#include <aurora/kdefs.h>
#include <boot/bootstructs.h>

/**
 * @brief Initializes the memory subsystem, including allocators, pre-defined pages, and so on. 
 * @param p_map Pointer to the memory map of the PC. This mainly concerns unmapped memory (such as the BIOS or APIC devices) and also gives us the info
 * as to where free memory is.
 * @param p_kernel_size The size of the kernel in bytes. Since the kernel resides at 0x0010 0000 in physical memory, we can use this to predict where our
 * allocators can start from. The kernel size is static once compiled, so we don't need to worry about this changing.
 * @return `true` if we could initialize the memory, and `false` if something went wrong. Any error here should be treated as failed, and cause an OS panic.
 */
bool initialize_memory(struct MemoryMap *p_map, uint32_t p_kernel_size);

/**
 * @brief Allocates N bytes of memory for use, with kernel-level permissions. Use whenever dynamic allocations are required.
 * Applications that require memory allocation should use `malloc` over this as the libk wrapper already handles these interactions.
 * @param p_size The number of bytes to allocate.
 * @return A pointer to the allocated memory if successful, and `NULL` if not.
 */
void *kalloc(uint32_t p_size);

/**
 * @brief Frees the memory from the allocator, if it is valid. If not, throws an error.
 * @param p_mem The memory region to free.
 */
void kfree(void *p_mem);

/**
 * @brief Maps a range of memory, usually that of memory-mapped peripherals, to a given virtual address. Preferred over calling `paging_map_region()`
 * as it checks in advance if the range is already being used by something else.
 * @param p_physical The physical address where memory is located
 * @param p_virtual The virtual address where memory is used
 * @param p_size The size of the memory to map (4 KiB aligned)
 * @return 
 */
bool kmap_range(uint32_t p_physical, uint32_t p_virtual, uint32_t p_size);

/**
 * @brief Checks to see if the address is aligned to a 4KiB page.
 * @param p_address The address to check
 * @return `TRUE` if yes, `FALSE` if not.
 */
bool is_4kib_aligned(void *p_address);

#endif // _AURORA_MEMORY_H