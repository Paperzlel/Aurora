#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PAGE_FLAG_GLOBAL = 1 << 8,
    PAGE_FLAG_SIZE_4MIB = 1 << 7,
    PAGE_FLAG_SIZE_4KIB = 0,
    PAGE_FLAG_USER = 1 << 2, 
    PAGE_FLAG_SUPERVISOR = 0,
    PAGE_FLAG_READ_WRITE = 1 << 1,
    PAGE_FLAG_READ_ONLY = 0,
} PagingFlags;

bool paging_map_region(void *p_from, void *p_to,  uint64_t p_size, PagingFlags p_flags);
void paging_free_region(void *p_address);

void paging_set_flag(void *p_address, PagingFlags p_flags);
void paging_clear_flag(void *p_address, PagingFlags p_flags);

uint32_t virtual_to_physical(uint32_t p_address);
