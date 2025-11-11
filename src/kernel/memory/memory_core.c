#include "memory_core.h"
#include "paging.h"

#include <stdio.h>

#define MAX_HANDLES 256

typedef struct {
    int device_handle_count;
    PageTableHandle handles[MAX_HANDLES];
} Memory_DeviceHandles;

static Memory_DeviceHandles handles;

bool initialize_memory_map(MemoryMap *p_map) {
    // Region-map every region
    for (int i = 0; i < p_map->region_count; i++) {
        MemoryRegion mr = p_map->regions[i];
        uint32_t mr_base = (uint32_t)mr.base_address;
        if (mr.type == MEMORY_REGION_ACPI_NVS || mr.type == MEMORY_REGION_RESERVED && !is_valid_address((void *)mr_base)) {
            if (!paging_map_region((void *)mr_base, (void *)mr_base, mr.length, &handles.handles[handles.device_handle_count - 1])) {
                printf("Failed to map region %x (size %x)\n", mr.base_address, mr.length);
                continue;
            }

            printf("Mapped region %llx of size %llx\n", mr.base_address, mr.length);
            // TODO: Add to reserved handle.
        }
    }

    return true;
}