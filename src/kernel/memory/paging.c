#include "paging.h"

#include "memdefs.h"
#include <memory.h>

#define PAGE_TABLE_COUNT PAGE_TABLE_MEMORY_SIZE / 4096
#define PAGE_TABLE_CONFIG_SIZE PAGE_TABLE_MEMORY_SIZE % 4096

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
    BIT_USERLAND = 1 << 2,
} PageInfoBits;

// (PAGE_TABLE_VIRTUAL_ADDRESS + PAGE_TABLE_MEMORY_SIZE - PAGE_TABLE_CONFIG_SIZE)

// HACK: We use this to reserve memory in the kernel for the page table config as it is otherwise unmapped memory. Move this at some point.
uint8_t __ptc_reserved[sizeof(PageTableConfig)] = { 0 };

PageTableConfig *config = (PageTableConfig *)&__ptc_reserved;

PageTable *allocated_tables = PAGE_TABLE_VIRTUAL_ADDRESS;
int used_table_count = 0;

// (PAGE_TABLE_VIRTUAL_ADDRESS + PAGE_TABLE_MEMORY_SIZE - PAGE_TABLE_CONFIG_SIZE) + sizeof(PageTableConfig)

uint8_t __pt_list_reserved[PAGE_TABLE_CONFIG_SIZE - sizeof(PageTableConfig)] = { 0 };

PageTableElement *pt_list_memory = (PageTableElement *)&__pt_list_reserved;
uint32_t pt_list_allocated[8];
int allocated_lists = 0;


extern uint32_t page_directory[1024];

void __attribute__((cdecl)) __tlb_flush(void *p_address);


// Linked-list implementation for memory:
// - Allocate new list
// - Free list (invalidate PT)
// - Append new list

/**
 * @brief Obtains the next immediately available page table index by finding the next "available" bit index and offset. Memory efficient but CPU-expensive.
 * @param out_list_idx The index for out page table to look for this value at.
 * @param out_offset The offset bit into the page table to look for the value at
 */
void _get_next_pt_index_and_offset(uint8_t *out_list_idx, uint8_t *out_offset) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < sizeof(uint32_t) * 8; j++) {
            if (!((pt_list_allocated[i] >> j) & 1)) {
                *out_list_idx = i;
                *out_offset = j;
                return;
            }
        }
    }
}

/**
 * @brief Sets the given offset and index value for the page table list as used.
 * @param p_index The index into the page table list array
 * @param p_offset The offset into the page table index bits
 */
void _set_pt_as_used(uint8_t p_index, uint8_t p_offset) {
    pt_list_allocated[p_index] |= 1 << p_offset;
}

/**
 * @brief Clears the given page table list index and marks it usable.
 * @param p_index The index into the page table list
 * @param p_offset The offset into the page table list bitmask
 */
void _set_pt_as_clear(uint8_t p_index, uint8_t p_offset) {
    pt_list_allocated[p_index] ^= 1 << p_offset;
}

bool _pt_list_check_used(uint8_t p_index, uint8_t p_offset) {
    return (pt_list_allocated[p_index] & 1 << p_offset) != 0;
}

/**
 * @brief Allocates a new page table list element from our given memory to use in a list. Linked-list information should only be handled by the code and
 * not touched by the user. In other words, only use the page table this gives in code.
 * @return `NULL` if the function was unable to allocate memory, and the pointer to the element if allocated.
 */
PageTableElement *_alloc_new_pt_list_element() {
    // Check for if the page table is overlapping into kernel memory. This is a serious case and should generally mean we move this information, but
    // is unlikely to be causes before we need more page tables anyways.
    // Include the potential table in this calculation.
    // if ((void *)(pt_list_memory + (allocated_lists * sizeof(PageTableElement)) + 1) >= KERNEL_VIRTUAL_ADDRESS) {
    //     return NULL;
    // }

    // Index * 32 + offset = index into array
    uint8_t idx, ofs;
    _get_next_pt_index_and_offset(&idx, &ofs);

    PageTableElement *ret = &pt_list_memory[idx * 32 + ofs];
    if (!ret) {
        return NULL;
    }

    ret->packed_idx_ofs = (idx << 8) + ofs;
    _set_pt_as_used(idx, ofs);
    allocated_lists++;
    return ret;
}

/**
 * @brief Frees a page table list element from memory. Only destroys the element itself and does not remove it from the list.
 * @param p_element The element to free
 */
void _pt_list_element_free(PageTableElement *p_element) {
    // Unpack index and offset
    uint8_t idx, ofs;
    idx = p_element->packed_idx_ofs >> 8;
    ofs = p_element->packed_idx_ofs & 0x00ff;

    if (!_pt_list_check_used(idx, ofs)) {
        return;
    }

    _set_pt_as_clear(idx, ofs);
    memset(p_element, 0, sizeof(PageTableElement));         // Clear used memory
    allocated_lists--;
    p_element = NULL;
}

/**
 * @brief Pushes an item to the end of the list. 
 * @param p_list The list to use
 * @param p_item The header item to use
 */
void _pt_list_push(PageTableElement *p_list, PageTableElement *p_item) {
    PageTableElement *end = p_list;
    while (end->next) {
        end = p_list->next;
    }

    end->next = p_item;
}

void _pt_list_pop(PageTableElement *p_list, PageTableElement *p_elem) {
    PageTableElement *item = p_list;
    PageTableElement *prev = p_list;
    while (item != p_elem && item) {
        item = item->next;
        prev = item;
    }

    if (item == NULL) {
        return;
    }

    if (prev) {
        prev->next = item->next;
    }
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
    if (idx == -1) {
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
    uint8_t idx = -1;
    for (int i = 0; i < PAGE_TABLE_COUNT; i++) {
        if (&allocated_tables[i] == p_table) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        return;
    }

    memset(p_table, 0, sizeof(PageTable));
    config->page_info[idx] ^= BIT_USED | BIT_KERNEL;
    config->available_space += 4096;
    config->available_tables++;
    used_table_count--;
}

bool paging_map_region(void *p_physical, void *p_virtual, uint32_t p_size, PageTableHandle *out_handle) {
    if (!config->intialized) {
        _init_paging_config();
    }

    // Find the number of directories to use
    int directory_count = p_size / (0x1000 * 0x1000);
    directory_count++;          // Always increment division

    void *phys_address = p_physical;
    uint32_t phys = (uint32_t)p_physical;
    uint16_t page_index = ((uint32_t)p_virtual & 0xffc00000) >> 22;

    PageTableElement *list = NULL;

    for (int i = 0; i < directory_count; i++) {
        // TODO: set an error
        if (used_table_count >= PAGE_TABLE_COUNT) {
            break;
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

        // Assign new PT list element
        PageTableElement *tmp = _alloc_new_pt_list_element();
        if (!tmp) {
            return false;
        }

        tmp->table = table;
        if (!list) {
            list = tmp;
        } else {
            _pt_list_push(list, tmp);
        }

        uint16_t table_start = 0;
        if (i <= 0) {
            table_start = ((uint32_t)p_virtual & 0x003ff000);
        }

        uint16_t table_end = 1024;
        if (i == directory_count - 1) {
            table_end = ((p_size % 0x400000) / 4096) + 1;
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
        uint32_t pde = (uint32_t)pt_phys & 0xfffff000 | 3;

        // Set PDE here to avoid invalid entries
        page_directory[page_index + i] = pde;
    }

    out_handle->size = p_size;
    out_handle->memory_start = p_virtual;
    out_handle->initialized = true;
    out_handle->internal = (void *)list;

    return true;
}

/**
 * @brief Frees the data associated to the given handle. Handles should not be created manually as they are generated by `paging_map_region()`.
 * @param p_handle The handle to the page table to modify.
 */
void paging_free_region(PageTableHandle *p_handle) {
    PageTableElement *root = (PageTableElement *)p_handle->internal;

    int i = 0;

    while (root) {
        if (root->table) {
            _pt_free(root->table);
        }

        uint16_t page_index = ((uint32_t)p_handle->memory_start & 0xffc00000) >> 22;
        page_directory[page_index + i] &= 2;            // Clear all bits except bit 1 (r/w bit)

        PageTableElement *this = root;
        root = root->next;
        _pt_list_element_free(this);
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