#include "memdetect.h"

#include "x86.h"
#include "memory.h"
#include "stdio.h"

#define MEMORY_MAX_REGIONS 256

struct MemoryRegion a_regions[MEMORY_MAX_REGIONS];
int a_region_count;

/**
 * @brief Obtains a map of the physical upper memory (i.e. all memory address >1 MiB). 
 * @param p_out_map The desired output map for the memory map.
 * @returns True if the function was able to get a proper map, and false if not.
 */
bool memory_get_mem_map(struct MemoryMap *p_out_map)
{
    struct MemoryRegion region;
    region.extended_attribs = 1;

    uint16_t current_region = 0;
    uint16_t next_region = 1;

    while (next_region != 0)
    {
        uint8_t size = x86_Memory_GetMemoryRegion(current_region, &next_region, &region);

        if (size == 255)
        {
            printf("Memory: Attempting to get map region #%d failed.\n", next_region);
            return false;
        }

        // Ignore region
        if (region.length == 0)
        {
            continue;
        }

        a_regions[a_region_count].base_address = region.base_address;
        a_regions[a_region_count].length = region.length;
        a_regions[a_region_count].type = region.type;
        a_regions[a_region_count].extended_attribs = (region.extended_attribs != 0 ? region.extended_attribs : 1);

        current_region = next_region;
        a_region_count++;

        printf("Memory: region found, base=0x%llx length=0x%llx type=0x%x\n", region.base_address, region.length, region.type);
    }
    
    // Manually add regions to the list that aren't present already
#define AUR_BOOT_ADD_REGION(m_base, m_size)                         \
        a_regions[a_region_count].base_address = m_base;            \
        a_regions[a_region_count].length = m_size;                  \
        a_regions[a_region_count].type = MEMORY_REGION_RESERVED;    \
        a_regions[a_region_count].extended_attribs = 1;             \
        a_region_count++

    AUR_BOOT_ADD_REGION(0xa0000, 0x50000);          // Manually block the bootloader
    AUR_BOOT_ADD_REGION(0x7c00, 0x200);             // Block the boot sector (we need the data in the BPB)

    p_out_map->regions = a_regions;
    p_out_map->region_count = a_region_count;

    return true;
}
