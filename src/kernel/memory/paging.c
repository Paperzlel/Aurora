#include "paging.h"

#include "memdefs.h"
#include <string.h>

#define PAGE_TABLE_COUNT (PAGE_TABLE_MEMORY_SIZE / 4096) - 1
#define PAGE_TABLE_CONFIG_SIZE (PAGE_TABLE_MEMORY_SIZE % 4096)

#define NULL ((void *)0)

typedef struct {
    uint32_t entry[1024];
} __attribute__((aligned(4096))) PageTable;

typedef struct _PageTableElement {
    struct _PageTableElement *next;

    PageTable *table;
    uint16_t packed_idx_ofs;
} PageTableElement;

typedef struct {
    uint8_t page_info[PAGE_TABLE_COUNT];
    PageTable *next_free;
    uint32_t available_space;
    uint8_t available_tables;
    bool intialized;
} PageTableConfig;

// Only use the first 3 bits so far for information, may use the rest at some point.
typedef enum {
    BIT_AVAILABLE = 0,
    BIT_USED = 1 << 0,
    BIT_PARTIALLY_FILLED = 1 << 1,
    BIT_KERNEL = 0 << 2,
    BIT_USERSPACE = 1 << 2,
} PageInfoBits;

// (PAGE_TABLE_VIRTUAL_ADDRESS + PAGE_TABLE_MEMORY_SIZE - PAGE_TABLE_CONFIG_SIZE)

// HACK: We use this to reserve memory in the kernel for the page table config as it is otherwise unmapped memory. Move this at some point.
uint8_t __ptc_reserved[sizeof(PageTableConfig)] = { 0 };

PageTableConfig *config = (PageTableConfig *)&__ptc_reserved;

PageTable *allocated_tables = PAGE_TABLE_VIRTUAL_ADDRESS;
int used_table_count = 0;

extern uint32_t page_directory[1024];

void __attribute__((cdecl)) __tlb_flush(void *p_address);

// Utility function (rounds up)
uint32_t ceil(uint32_t x, uint32_t y) {
    return x % y == 0 ? x / y : (x / y) + 1;
}


void _init_paging_config() {
    config->available_space = PAGE_TABLE_MEMORY_SIZE - PAGE_TABLE_CONFIG_SIZE;
    config->available_tables = PAGE_TABLE_COUNT;
    config->next_free = allocated_tables;
    config->intialized = true;
}

PageTable *_alloc_new_table() {
    if (config->available_tables == 0) {
        return NULL;
    }

    // Check bits in page table config for our needed table
    uint16_t idx = -1;
    for (int i = 0; i < PAGE_TABLE_COUNT; i++) {
        if (!(config->page_info[i] & BIT_USED)) {
            idx = i;
            break;
        }
    }

    // Failed to find a valid table (potential overload?)
    if (idx == (uint16_t)-1) {
        return NULL;
    }

    // Assign next table
    PageTable *ret = config->next_free;
    config->available_tables--;
    config->available_space -= 4096;
    // Set active
    config->page_info[idx] = BIT_USED | BIT_KERNEL;

    for (int i = idx; i < PAGE_TABLE_COUNT; i++) {
        if (!(config->page_info[i] & 1)) {
            config->next_free = &allocated_tables[i];
            used_table_count++;
            break;
        }
    }

    return ret;
}

void _pt_free(PageTable *p_table) {
    uint16_t idx = -1;
    for (int i = 0; i < PAGE_TABLE_COUNT; i++) {
        if (&allocated_tables[i] == p_table) {
            idx = i;
            break;
        }
    }

    if (idx == (uint16_t)-1) {
        return;
    }

    memset(p_table, 0, sizeof(PageTable));
    config->page_info[idx] ^= BIT_USED | BIT_KERNEL;
    config->available_space += 4096;
    config->available_tables++;
    used_table_count--;
}

// 1024 PT entries --> 4096 KiB = 4 MiB = 0x400000 in hex per PT
// 1024 PD entries --> 4096 * 1024 = 4194304 KiB = 4096 MiB = 4 GiB

/**
 * @brief Finds the number of directories to use for the given virtual address and size. Virtual addresses that are not aligned to a 4 MiB
 * boundary may straddle a border, which doesn't get picked up by the `ceil()` function. In order to find the appropriate number of directories,
 * we need to check if the remaining address space until the next 4 MiB boundary is greater than the size, and if so, we subtract the difference
 * and ceiling-divide size to get the directory count.
 * @param p_virtual The base virtual address to place the memory at
 * @param p_size The amount of memory we are looking to map
 * @return The number of page directories to allocate.
 */
int _get_directory_count(uint32_t p_virtual, uint32_t p_size) {
    uint32_t remainder_in_virt = (p_virtual & 0xffc00000) + 0x400000 - p_virtual;
    // Isn't straddling a border
    if (p_size <= remainder_in_virt) {
        return 1;
    }

    p_size -= remainder_in_virt;
    return ceil(p_size, 0x400000) + 1;
}

bool paging_map_region(void *p_physical, void *p_virtual, uint32_t p_size) {
    if (!config->intialized) {
        _init_paging_config();
    }

    // Already mapped, no need to remap
    if (is_valid_range(p_virtual, p_virtual + p_size)) {
        return true;
    }

    int directory_count = _get_directory_count((uint32_t)p_virtual, p_size);
    void *phys_address = p_physical;
    uint16_t page_index = ((uint32_t)p_virtual & 0xffc00000) >> 22;

    for (int i = 0; i < directory_count; i++) {
        // TODO: set an error
        if (used_table_count >= PAGE_TABLE_COUNT) {
            return false;
        }

        PageTable *table = NULL;

        // Page index is unused, allocate a page index
        if (!(page_directory[page_index + i] & 1)) {
            // Need a new table, get one
            table = _alloc_new_table();
        } else {
            // Flush TLB to update after availability changes
            __tlb_flush(p_virtual);
            table = (PageTable *)(page_directory[page_index + i] & 0xfffff000);
        }

        // TODO: Throw an error of some kind
        if (!table) {
            return false;
        }

        uint16_t table_start = 0;
        uint16_t table_end = 1024;
        if (i == 0) {
            table_start = ((uint32_t)p_virtual & 0x003ff000) >> 12;
        } 
        if (i == directory_count - 1) {
            // End of table must be 1 less than the full size as here we map 4096 bytes INCLUDING byte 0.
            table_end = ceil((((uint32_t)p_virtual + p_size - 1) % 0x400000), 4096);
            table_end = (table_end > 1024) ? 1024 : table_end;
        }

        for (int j = table_start; j < table_end; j++, phys_address += 4096) {
            // TODO: Some kind of error message here
            if (table->entry[j] != 0) {
                return false;
            }

            uint32_t ptr = ((uint32_t)phys_address & 0xfffff000) | 3;
            table->entry[j] = ptr;
        }

        uint32_t pt_phys = virtual_to_physical((uint32_t)table);
        uint32_t pde = ((uint32_t)pt_phys & 0xfffff000) | 3;

        // Set PDE here to avoid invalid entries
        page_directory[page_index + i] = pde;
    }

    return true;
}

void paging_free_region(void *p_virtual, uint32_t p_size) {
    int directory_count = _get_directory_count((uint32_t)p_virtual, p_size);

    for (int i = 0; i < directory_count; i++) {
        uint16_t page_index = ((uint32_t)p_virtual & 0xffc00000) >> 22;
        PageTable *table = (PageTable *)(page_directory[page_index + i] & 0xfffff000);
        page_directory[page_index + i] &= 2;            // Clear all bits except bit 1 (r/w bit)

        uint16_t table_start = 0;
        uint16_t table_end = 1024;
        if (i == 0) {
            table_start = ((uint32_t)p_virtual & 0x003ff000) >> 12;
        }
        if (i == directory_count - 1) {
            table_end = ceil((((uint32_t)p_virtual + p_size - 1) % 0x400000), 4096);
            table_end = (table_end > 1024) ? 1024 : table_end;
        }

        // Can free entire page table
        bool free_pt = (table_start == 0 && table_end == 1024);

        for (int j = table_start; j < table_end; j++) {
            table->entry[j] = 0;
        }

        // Check if lower regions are empty as well
        bool lower_empty = false;
        if (table_start > 0) {
            for (int i = 0; i < table_start; i++) {
                if (table->entry[i] != 0) {
                    lower_empty = false;
                    break;
                }
            }

            lower_empty = true;
        }

        // Check if upper regions are empty as well
        bool upper_empty = false;
        if (table_end < 1024) {
            for (int i = table_end; i < 1024; i++) {
                if (table->entry[i] != 0) {
                    upper_empty = false;
                    break;
                }
            }

            upper_empty = true;
        }

        // If both lower and upper are empty, free the table.
        if (lower_empty && upper_empty) {
            free_pt = true;
        }

        if (free_pt) {
            _pt_free(table);
        }
    }
}


uint32_t virtual_to_physical(uint32_t p_virtual) {
    uint16_t dir = (p_virtual & 0xffc00000) >> 22;
    PageTable *pt = (PageTable *)(page_directory[dir] & 0xfffff000);
    if (!pt || !(page_directory[dir] & 1)) {
        return 0;
    }
    
    uint16_t idx = (p_virtual & 0x003ff000) >> 12;
    uint32_t ret = pt->entry[idx] & 0xfffff000;
    if (!ret || !(pt->entry[idx] & 1)) {
        return 0;
    }

    return ret + (p_virtual & 0x00000fff);
}

bool is_valid_address(void *p_virtual) {
    return virtual_to_physical((uint32_t)p_virtual) != 0;
}

bool is_valid_range(void *p_start, void *p_end) {
    // Start and end may need to be mapped
    if (virtual_to_physical((uint32_t)p_start) == 0 || virtual_to_physical((uint32_t)p_end) == 0) {
        return false;
    }

    int range_size = (uint32_t)(p_end - p_start);
    while (range_size > 0) {
        if (virtual_to_physical((uint32_t)(p_end - range_size)) == 0) {
            return false;
        }

        range_size -= 4096;
    }

    return true;
}