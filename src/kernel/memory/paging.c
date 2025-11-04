#include "paging.h"

#include "memdefs.h"
#include <memory.h>

typedef struct {
    uint32_t entry[1024];
} __attribute__((aligned(4096))) PageTable;

PageTable *allocated_tables = PAGE_TABLE_VIRTUAL_ADDRESS;
int used_table_count;

const int PAGE_TABLE_COUNT = PAGE_TABLE_MEMORY_SIZE / 4096;
const int PAGE_TABLE_CONFIG_SIZE = PAGE_TABLE_MEMORY_SIZE % 4096;

extern uint32_t __attribute__((section(".kernel_lh.data")))  page_directory[1024];

void __attribute__((cdecl)) __tlb_flush(void *p_address);

void __attribute__((cdecl)) test(int idx, uint32_t value);

uint32_t virtual_to_physical(uint32_t p_virtual) {
    uint16_t dir = (p_virtual & 0xffc00000) >> 22;
    PageTable *pt = (PageTable *)(page_directory[dir] & 0xfffff000);
    if (!pt) {
        return 0;
    }
    uint16_t idx = (p_virtual & 0x003ff000) >> 12;
    uint32_t ret = pt->entry[idx] & 0xfffff000;
    return ret + (p_virtual & 0x00000fff);
}

bool paging_map_region(void *p_physical, void *p_virtual, uint32_t p_size) {
    // Find the number of directories to use
    int directory_count = p_size / 0x400000;
    if ((directory_count % 0x400000) != 0) {
        directory_count++;
    }

    int test = 5 / 2;
    test = 4 / 2;
    test = 3 / 2;

    void *phys_address = p_physical;
    uint32_t phys = (uint32_t)p_physical;
    uint16_t page_index = ((uint32_t)p_virtual & 0xffc00000) >> 22;

    for (int i = 0; i < directory_count; i++) {
        // TODO: set an error
        if (used_table_count >= PAGE_TABLE_COUNT) {
            break;
        }

        PageTable *table = ((void *)0);

        // Page index is unused, allocate a page index
        if (!(page_directory[page_index + i] & 1)) {
            // Need a new table, get one
            table = &allocated_tables[used_table_count];
            // Increment used table count
            used_table_count++;
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
        if (i <= 0) {
            table_start = ((uint32_t)p_virtual & 0x003ff000);
        }

        uint16_t table_end = 1024;
        if (i == directory_count - 1) {
            table_end = (p_size % 0x400000) / 4096;
        }

        for (int j = table_start; j < table_end; j++, phys_address += 4096) {
            uint32_t ptr = ((uint32_t)phys_address & 0xfffff000) | 3;
            table->entry[j] = ptr;
        }

        uint32_t pt_phys = virtual_to_physical((uint32_t)table);
        uint32_t pde = (uint32_t)pt_phys & 0xfffff000 | 3;

        // Set PDE here to avoid invalid entries
        page_directory[page_index + i] = pde;
    }

    return true;
}