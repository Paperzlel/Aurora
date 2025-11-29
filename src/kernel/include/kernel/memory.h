#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include <kernel/kdefs.h>

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

#endif // _KERNEL_MEMORY_H