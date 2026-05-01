#ifndef _AURORA_MEMORY_H
#define _AURORA_MEMORY_H

#include <aurora/kdefs.h>

#include <boot/bootstructs.h>

/**
 * @brief Initializes the memory subsystem, including allocators, pre-defined pages, and so on.
 * @param p_map Pointer to the memory map of the PC. This mainly concerns unmapped memory (such as the BIOS or APIC
 * devices) and also gives us the info as to where free memory is.
 * @param p_kernel_size The size of the kernel in bytes. Since the kernel resides at 0x0010 0000 in physical memory, we
 * can use this to predict where our allocators can start from. The kernel size is static once compiled, so we don't
 * need to worry about this changing.
 * @return `true` if we could initialize the memory, and `false` if something went wrong. Any error here should be
 * treated as failed, and cause an OS panic.
 */
bool initialize_memory(struct MemoryMap *p_map, uint32_t p_kernel_size);

/**
 * @brief Allocates N bytes of memory for use, with kernel-level permissions. Use whenever dynamic allocations are
 * required. Applications that require memory allocation should use `malloc` over this as the libk wrapper already
 * handles these interactions.
 * @param p_size The number of bytes to allocate.
 * @return A pointer to the allocated memory if successful, and `NULL` if not.
 */
void *kalloc(uint32_t p_size);

/**
 * @brief Modifies the amount of data pointed to by ptr to the new value passed in.
 * @param ptr The pointer to modify
 * @param p_size The number of bytes to reallocate to
 * @return The pointer to the new object, which may have changed.
 */
void *krealloc(void *ptr, size_t p_size);

/**
 * @brief Frees the memory from the allocator, if it is valid. If not, throws an error.
 * @param p_mem The memory region to free.
 */
void kfree(void *p_mem);

/**
 * @brief Maps a range of memory, usually that of memory-mapped peripherals, to a given virtual address. If the range is already in use, throws an
 * error and returns.
 * @param p_physical The physical address where memory is currently located.
 * @param p_virtual The virtual address where memory wants to appear as. Memory can thus appear non-contingous.
 * @param p_size The size of the memory to map (4 KiB aligned, hopefully.)
 * @return `true` on success, and `false` if the address range is already in use or if the pager was unable to map the memory properly.
 */
bool kmap_range(uint32_t p_physical, uint32_t p_virtual, uint32_t p_size, bool p_userspace_range);

/**
 * @brief Unmaps a given range, if the range is valid.
 * @param p_virtual The starting pointer of a given range of virtual memory to unmap.
 * @param p_size The number of bytes to unmap. Should be 4 KiB aligned.
 */
void kunmap_range(uint32_t p_virtual, size_t p_size);

/**
 * @brief Toggles whether a given page is a userspace or kernel page. Pages that are set to usermode can be run with lower priviledge, but be wary
 * of letting userspace actually get hold of this information. Ideally, these code blocks would never be interruptible.
 * @param p_virtual The virtual address to translate. Can exist inside of a page.
 * @param p_size The number of bytes to translate. Rounded from the initial address.
 * @param p_value Whether to enable a userspace page or revert it to the kernel.
 */
void ktoggle_page_usermode(uint32_t p_virtual, size_t p_size, bool p_value);

/**
 * @brief Creates a userspace heap for userspace code to use. Since program initialization needs a heap for process space, we do so here.
 * Other options will be added in future.
 * @return A pointer to the new heap structure on success, and `NULL` on failure. This pointer should be used for any userspace heap allocations
 * for the given process.
 */
void *kmake_user_heap();

/**
 * @brief Destroys and removes any data correlating to the given heap. Since there is no freeing yet, this does nothing and should not be used.
 * @param p_heap A pointer to the heap to destroy data for.
 */
void kdestroy_user_heap(void *p_heap);

/**
 * @brief Allocates N bytes of memory on the userspace heap provided.
 * @param p_heap The heap in question to allocate for. Held by the processor.
 * @param p_size The number of bytes to allocate.
 * @return A pointer to the heap on success, and `NULL` on failure.
 */
void *kuserheap_alloc(void *p_heap, size_t p_size);

/**
 * @brief Frees the data corresponding to the given pointer on the heap. The address must be a direct match to that of the allocated one for this 
 * to work.
 * @param p_heap The heap to free data from. Held in the process information.
 * @param p_ptr A pointer that was previously allocated.
 */
void kuserheap_free(void *p_heap, void *p_ptr);

/**
 * @brief Converts a mapped virtual address to a physical one.
 * @param p_address The address to convert
 * @return The physical address, or 0 if the address couldn't be found. Since the BIOS is identity-mapped, please don't
 * use it for that.
 */
uint32_t virtual_to_physical(uint32_t p_address);

/**
 * @brief Converts a physical address to its mapped counterpart, if it exists.
 * @param p_address The (physical) address to convert
 * @return The virtual address, or 0 if the physical address doesn't map correctly.
 */
uint32_t physical_to_virtual(uint32_t p_address);

/**
 * @brief Converts a mapped virtual address to a physical one, using `void *` instead of `uint32_t`.
 * @param p_address The address to convert.
 * @return The physical address, or 0 if the address couldn't be found.
 */
void *pvirtual_to_physical(void *p_virtual);

/**
 * @brief Checks to see if the given virtual address has a mapped physical address.
 * @param p_virtual The address to check for
 * @return `true` if yes, `false` if no.
 */
bool is_valid_address(void *p_virtual);

/**
 * @brief Checks to see if the given virtual address range is usable. Normally used in conjunction with other paging
 * functions to map a range if needed
 * @param p_start The start address in the range
 * @param p_end The end address in the range
 * @return `true` if yes, `false` if no.
 */
bool is_valid_range(uint32_t p_start, uint32_t p_end);

/**
 * @brief Checks to see if the address is aligned to a 4KiB page.
 * @param p_address The address to check
 * @return `TRUE` if yes, `FALSE` if not.
 */
bool is_4kib_aligned(void *p_address);

#endif // _AURORA_MEMORY_H