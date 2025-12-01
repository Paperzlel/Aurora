#include <kernel/memory.h>
#include <kernel/memdefs.h>
#include <boot/bootstructs.h>

#include "paging.h"

#define AUR_MODULE "memory"
#include <kernel/debug.h>

#include <stdio.h>
#include <string.h>

typedef enum {
    BIT_AVAILABLE = 1 << 0,
    BIT_USED = 0,
    BIT_KERNEL = 1 << 1,
    BIT_USERSPACE = 0,
} MemoryHeaderFlags;

typedef struct MemoryHeader {
    uint32_t virtual;
    uint32_t physical;
    uint32_t size;
    uint8_t flags;
    struct MemoryHeader *next;
    struct MemoryHeader *prev;
} MemoryHeader;


MemoryHeader *root_header = KERNEL_ALLOC_VIRTUAL_ADDRESS;

bool initialize_memory(MemoryMap *p_map, uint32_t p_kernel_size) {
    // Already mapped memory, return
    if (is_valid_address(root_header) && root_header->size != 0) {
        return true;
    }

    MemoryRegion usable_mr = { 0 }; 
    bool found = false;
    uint64_t blocked_ranges[16] = { 0 };        // Block a max of 8 ranges where physical memory cannot be used
    uint8_t ranges = 0;

    // Region-map every region
    int i = 0;
    for (i = 0; i < p_map->region_count; i++) {
        MemoryRegion mr = p_map->regions[i];
        uint32_t mr_base = (uint32_t)mr.base_address;
        if (mr.type == MEMORY_REGION_ACPI_NVS || (mr.type == MEMORY_REGION_RESERVED && !is_valid_address((void *)mr_base))) {
            if (!paging_map_region((void *)mr_base, (void *)mr_base, mr.length)) {
                LOG_ERROR("Failed to map region %x (size %x)\n", mr.base_address, mr.length);
                continue;
            }
        }

        if (mr.type == MEMORY_REGION_USABLE && mr_base >= KERNEL_PHYSICAL_ADDRESS && !found) {
            found = true;
            usable_mr = mr;
        }

        if (mr.type != MEMORY_REGION_USABLE && usable_mr.base_address + usable_mr.length > mr_base) {
            blocked_ranges[ranges * 2] = mr.base_address;
            blocked_ranges[(ranges * 2) + 1] = mr.length;
            ranges++;
        }

        if (ranges > 8) {
            LOG_ERROR("Memory: Cannot have more than 8 blocked ranges, aborting...\n");
            return false;
        }

        LOG_DEBUG("Region start 0x%llx, region end 0x%llx\n", mr.base_address, mr.base_address + mr.length);
    }

    // Align kernel size to next 16 bytes
    p_kernel_size += (0x10 - (p_kernel_size % 0x10)) % 0x10;

    // Create first header
    uint32_t max_address = usable_mr.base_address + usable_mr.length;
    MemoryHeader mh = { 0 };
    mh.physical = usable_mr.base_address + p_kernel_size;
    mh.virtual = (uint32_t)KERNEL_ALLOC_VIRTUAL_ADDRESS;
    mh.size = usable_mr.length - p_kernel_size - sizeof(MemoryHeader);
    mh.prev = NULL;
    mh.next = NULL;
    mh.flags = BIT_AVAILABLE | BIT_KERNEL;
    if (!paging_map_region((void *)mh.physical, KERNEL_ALLOC_VIRTUAL_ADDRESS, 0x1000)) {
        LOG_ERROR("Failed to map the first memory header to its virtual address.\n");
        return false;
    }
    // Commit memory header once done
    memcpy(KERNEL_ALLOC_VIRTUAL_ADDRESS, &mh, sizeof(MemoryHeader));
    
    // Virtual memory uses the process of offset 0 being KERNEL_ALLOC_VIRTUAL_ADDRESS and each size + blocked_size is the next offset
    
    uint32_t offset = 0;
    MemoryHeader *ptr = (MemoryHeader *)KERNEL_ALLOC_VIRTUAL_ADDRESS;
    if (ranges > 0) {
        mh.size = blocked_ranges[0] - mh.physical - sizeof(MemoryHeader);
        // Per sub-range, create a link to the next available memory slot (the slot after the specified memory range)
        for (i = 0; i < ranges; i++) {
            MemoryHeader sub_mh;
            // Physical address is after the blocked region
            sub_mh.physical = blocked_ranges[2 * i] + blocked_ranges[(2 * i) + 1];
            // Size is either the difference between the current address and the next blocked address, or the difference between the max address and the
            // current physical address if at the end of the system.
            sub_mh.size = (blocked_ranges[2 * (i + 1)] > 0 ? blocked_ranges[2 * (i + 1)] - sub_mh.physical : max_address - sub_mh.physical) - sizeof(MemoryHeader);
            // Flags are always set to be kernel memory and available
            sub_mh.flags = BIT_AVAILABLE | BIT_KERNEL;
            
            // Set offset
            offset += ptr->size + blocked_ranges[(2 * i) + 1] + sizeof(MemoryHeader);
            if (!paging_map_region((void *)sub_mh.physical, KERNEL_ALLOC_VIRTUAL_ADDRESS + offset, 0x1000)) {
                printf("Failed to map sub-region of memory, aborting...\n");
                return false;
            }
            sub_mh.virtual = (uint32_t)KERNEL_ALLOC_VIRTUAL_ADDRESS + offset;
            // Commit memory header
            memcpy((void *)sub_mh.virtual, &sub_mh, sizeof(MemoryHeader));
            MemoryHeader *this = (MemoryHeader *)sub_mh.virtual;
            // Setup linked-list
            ptr->next = this;
            this->prev = ptr;
            
            ptr = this;
        }
    }

    MemoryHeader *root = root_header;
    while (root) {
        printf("Available memory region: Base address %x | end %x\n", root->virtual, root->size + root->virtual);
        root = root->next;
    }

    return true;
}

void *kalloc(uint32_t p_size) {
    MemoryHeader *list = root_header;
    while (list) {
        if (list->flags & BIT_AVAILABLE && list->size > p_size) {
            break;
        }
    }

    // All lists are occupied (should never happen)
    if (!list) {
        printf("ERROR: OUT OF MEMORY EXCEPTION.\n");
        return NULL;
    }

    // Insert new region after the now-used one
    MemoryHeader new_mh = { 0 };
    new_mh.physical = list->physical + p_size + sizeof(MemoryHeader);
    new_mh.virtual = list->virtual + p_size + sizeof(MemoryHeader);
    new_mh.flags = BIT_AVAILABLE | BIT_KERNEL;
    new_mh.prev = list;
    new_mh.size = list->size - (p_size + sizeof(MemoryHeader));

    // Map header and 4KiB region as usable
    if (!is_valid_address((void *)new_mh.virtual)) {
        if (!paging_map_region((void *)new_mh.physical, (void *)new_mh.virtual, 0x1000)) {
            printf("ERROR: Could not map the new memory header.\n");
            return NULL;
        }
    }

    memcpy((void *)new_mh.virtual, &new_mh, sizeof(MemoryHeader));
    MemoryHeader *new_ptr = (MemoryHeader *)new_mh.virtual;

    list->size = p_size;
    if (list->next) {
        list->next->prev = new_ptr;
        new_ptr->next = list->next;
    }
    list->next = new_ptr;
    list->flags &= ~BIT_AVAILABLE;       // Clear used flag

    // Map new region (returns early if already mapped)
    if (!paging_map_region((void *)list->physical, (void *)list->virtual, list->size)) {
        printf("ERROR: Could not map the new memory region.\n");
        return NULL;
    }

    return (void *)(list->virtual + sizeof(MemoryHeader));
}

void kfree(void *p_mem) {
    MemoryHeader *list = root_header;
    while (list) {
        if (list->virtual == ((uint32_t)(p_mem - sizeof(MemoryHeader)))) {
            break;
        }
    }

    // TODO: Should segfault here.
    if (!list || list->flags & BIT_AVAILABLE) {
        printf("ERROR: pretend this is a segfault. Double free occured.\n");
        return;
    }
    
    // Remove the element from the list. All memory used goes to the previous allocated block to defragment the memory
    if (list->prev) {
        list->prev->next = list->next;
        list->prev->size += list->size + sizeof(MemoryHeader);      // MemoryHeader takes an extra few bytes to exist, which can be used up.
    } else {
        // Freed memory at the root header. Don't remove it, just mark as clear and move on.
        list->flags |= BIT_AVAILABLE;
        return;
    }

    if (list->next) {
        list->next->prev = list->prev;
    }

    // Region is now removed from memory. Since we expect allocators to look after raw memory and whether its garbage or not, leave it as-is.
    // Free the memory region now since it may have tables that shouldn't be used.
    paging_free_region((void *)list->virtual, list->size);
}


bool kmap_range(void *p_physical, void *p_virtual, uint32_t p_size) {
    //TODO: Add more here
    return paging_map_region(p_physical, p_virtual, p_size);
}


bool is_4kib_aligned(void *p_address) {
    return ((uint32_t)p_address & 0xfffff000) == (uint32_t)p_address;
}
