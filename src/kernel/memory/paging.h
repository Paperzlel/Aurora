#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void *internal;                 // Internal data of the page table
    void *memory_start;             // Start of the memory region
    uint32_t size;                  // Number of bytes in memory
    bool initialized;               // Whether the handle has been initialized
} PageTableHandle;

typedef enum {
    PAGE_FLAG_GLOBAL = 1 << 8,
    PAGE_FLAG_SIZE_4MIB = 1 << 7,
    PAGE_FLAG_SIZE_4KIB = 0,
    PAGE_FLAG_USER = 1 << 2, 
    PAGE_FLAG_SUPERVISOR = 0,
    PAGE_FLAG_READ_WRITE = 1 << 1,
    PAGE_FLAG_READ_ONLY = 0,
} PagingFlags;


bool paging_map_region(void *p_physical, void *p_virtual, uint32_t p_size, PageTableHandle *out_handle);
void paging_free_region(PageTableHandle *p_handle);

uint32_t virtual_to_physical(uint32_t p_address);
bool is_valid_address(void *p_virtual);