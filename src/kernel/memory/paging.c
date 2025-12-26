#include "paging.h"

#include <aurora/memdefs.h>
#include <string.h>

#define AUR_MODULE "paging"
#include <aurora/debug.h>

#define PAGE_TABLE_COUNT (PAGE_TABLE_MEMORY_SIZE / 4096)

#define MIN_VIRTUAL_ADDRESS_LOCATION 

struct __attribute__((aligned(4096))) PageTable
{
    uint32_t entry[1024];
};

struct PageTableConfig
{
    uint8_t page_info[PAGE_TABLE_COUNT];
    struct PageTable *next_free;
    uint32_t available_space;
    uint16_t available_tables;
    uint32_t kernel_vmem_start;                     // The point where kernel virtual memory can be allocated from
    uint32_t user_vmem_start;                       // The point where userspace virtual memory can be allocated from
};

// Only use the first 3 bits so far for information, may use the rest at some point.
enum PageInfoBits
{
    BIT_AVAILABLE = 0,
    BIT_USED = 1 << 0,
    BIT_PARTIALLY_FILLED = 1 << 1,
    BIT_KERNEL = 0 << 2,
    BIT_USERSPACE = 1 << 2,
};

// (PAGE_TABLE_VIRTUAL_ADDRESS + PAGE_TABLE_MEMORY_SIZE - PAGE_TABLE_CONFIG_SIZE)

// HACK: We use this to reserve memory in the kernel for the page table config as it is otherwise unmapped memory. Move this at some point.
static uint8_t __ptc_reserved[sizeof(struct PageTableConfig)] = { 0 };

const int x = sizeof(struct PageTableConfig);

struct PageTableConfig *config = (struct PageTableConfig *)&__ptc_reserved;
struct PageTable *allocated_tables = (void *)PAGE_TABLE_VIRTUAL_ADDRESS;
int used_table_count = 0;

extern uint32_t page_directory[1024];
extern uint8_t __end;                       // Address at the end of the kernel

void __attribute__((cdecl)) __tlb_flush(void *p_address);


// Utility function (rounds up)
uint32_t ceil(uint32_t x, uint32_t y)
{
    return x % y == 0 ? x / y : (x / y) + 1;
}


struct PageTable *_alloc_new_table()
{
    if (config->available_tables == 0)
    {
        return NULL;
    }

    // Check bits in page table config for our needed table
    uint16_t idx = -1;
    for (int i = 0; i < PAGE_TABLE_COUNT; i++)
    {
        if (!(config->page_info[i] & BIT_USED))
        {
            idx = i;
            break;
        }
    }

    // Failed to find a valid table (potential overload?)
    if (idx == (uint16_t)-1)
    {
        return NULL;
    }

    // Assign next table
    struct PageTable *ret = config->next_free;
    config->available_tables--;
    config->available_space -= 4096;
    // Set active
    config->page_info[idx] = BIT_USED | BIT_KERNEL;

    for (int i = idx; i < PAGE_TABLE_COUNT; i++)
    {
        if (!(config->page_info[i] & 1))
        {
            config->next_free = &allocated_tables[i];
            used_table_count++;
            break;
        }
    }

    return ret;
}


void _pt_free(struct PageTable *p_table)
{
    uint16_t idx = -1;
    for (int i = 0; i < PAGE_TABLE_COUNT; i++)
    {
        if (&allocated_tables[i] == p_table)
        {
            idx = i;
            break;
        }
    }

    if (idx == (uint16_t)-1)
    {
        return;
    }

    memset(p_table, 0, sizeof(struct PageTable));
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
int _get_directory_count(uint32_t p_virtual, uint32_t p_size)
{
    uint32_t remainder_in_virt = (p_virtual & 0xffc00000) + 0x400000 - p_virtual;
    // Isn't straddling a border
    if (p_size <= remainder_in_virt)
    {
        return 1;
    }

    p_size -= remainder_in_virt;
    return ceil(p_size, 0x400000) + 1;
}


uint32_t find_next_free_region(uint32_t p_size)
{
    uint32_t ret = config->kernel_vmem_start;
    int i = 0, j = 0;
    struct PageTable *pt = NULL;
    for (i = (ret & 0xffc00000) >> 22; i < 1024; i++)
    {
        pt = (struct PageTable *)(page_directory[i] & 0xfffff000);
        if (!pt)
        {
            ret = i << 22;
            break;
        }

        bool found = false;
        for (j = 0; j < 1024; j++)
        {
            if (pt->entry[j] == 0)
            {
                ret = (i << 22) | (j << 12);
                found = true;
                break;
            }
        }

        if (found) break;
    }

    // Validate if the needed value of page directories can use the whole address space. It doesn't matter if separate blocks are non-contingous, but
    // per-allocation blocks MUST be in order for the system to work. Returning NULL here allows the code to catch that an allocation failed for
    // this specific reason.
    int count = _get_directory_count(ret, p_size);
    if (pt)
    {
        for (int k = j; k < 1024; k++)
        {
            if (pt->entry[k] != 0)
            {
                LOG_WARNING("Memory region mappign to region %x was blocked by other allocation %x.", ret, pt->entry[k]);
                return 0;
            }
        }
    }

    if (count > 1)
    {
        for (int k = i + 1; k < i + count; k++)
        {
            if (!(page_directory[k] & 0xfffff000)) continue;

            pt = (struct PageTable *)(page_directory[k] & 0xfffff000);
            for (int l = 0; l < 1024; l++)
            {
                if (pt->entry[l] != 0)
                {
                    LOG_WARNING("Memory region mapping to region %x was blocked by other allocation %x.", ret, pt->entry[l]);
                    return 0;
                }
            }
        }
    }

    return ret;
}

void paging_initialize(uint32_t *p_phys_mem_start)
{
    config->available_space = PAGE_TABLE_MEMORY_SIZE;
    config->available_tables = PAGE_TABLE_COUNT;
    config->next_free = allocated_tables;

    // NOTE: This is the first (non-aligned) memory address available after the end of the kernel. 
    config->kernel_vmem_start = (uint32_t)&__end;
    config->user_vmem_start = USER_ALLOC_VIRTUAL_ADDRESS;

    uint32_t additional_mem_size = KERNEL_VIRTUAL_ADDRESS - 0xc00f0000;
    if (!paging_map_region(*p_phys_mem_start, 0xc00f0000, additional_mem_size))
    {
        LOG_FATAL("Failed to initialize paging, unable to plug memory hole.");
    }
    // Add additionally allocated memory in.
    *p_phys_mem_start += additional_mem_size;
}


bool paging_map_region(uint32_t p_physical, uint32_t p_virtual, uint32_t p_size)
{
    // Already mapped, no need to remap
    if (is_valid_range(p_virtual, p_virtual + p_size))
    {
        return true;
    }

    int directory_count = _get_directory_count(p_virtual, p_size);
    uint16_t page_index = (p_virtual & 0xffc00000) >> 22;

    for (int i = 0; i < directory_count; i++)
    {
        struct PageTable *table = NULL;

        // Page index is unused, allocate a page index
        if (!(page_directory[page_index + i] & 1))
        {
            if (used_table_count >= PAGE_TABLE_COUNT)
            {
                LOG_ERROR("Number of tables used now exceeds the valid page table count. The mapped memory may not be entirely usable.s");
                return false;
            }

            // Need a new table, get one
            table = _alloc_new_table();
        }
        else
        {
            // Flush TLB to update after availability changes
            __tlb_flush((void *)p_virtual);
            table = (struct PageTable *)(page_directory[page_index + i] & 0xfffff000);
        }

        if (!table)
        {
            LOG_ERROR("No available page tables for mapping memory. The mapped memory range may not be entirely usable.");
            return false;
        }

        uint16_t table_start = 0;
        uint16_t table_end = 1024;
        if (i == 0)
        {
            table_start = ((uint32_t)p_virtual & 0x003ff000) >> 12;
        } 

        if (i == directory_count - 1)
        {
            // End of table must be 1 less than the full size as here we map 4096 bytes INCLUDING byte 0.
            table_end = ceil((((uint32_t)p_virtual + p_size - 1) % 0x400000), 4096);
            table_end = (table_end > 1024) ? 1024 : table_end;
        }

        for (int j = table_start; j < table_end; j++, p_physical += 4096)
        {
            if (table->entry[j] != 0)
            {
                LOG_ERROR("Table entry found within memory region range. The mapped memory range may not be entirely usable.");
                return false;
            }

            uint32_t ptr = (p_physical & 0xfffff000) | 3;
            table->entry[j] = ptr;
        }

        uint32_t pt_phys = virtual_to_physical((uint32_t)table);
        uint32_t pde = ((uint32_t)pt_phys & 0xfffff000) | 3;

        // Set PDE here to avoid invalid entries
        page_directory[page_index + i] = pde;
    }

    return true;
}


void *paging_allocate_region(uint32_t p_address, uint32_t p_size)
{
    uint32_t virtual = physical_to_virtual(p_address);
    if (virtual && is_valid_range(virtual, virtual + p_size))
    {
        LOG_WARNING("Requested region %x is already mapped to virtual memory.", p_address);
        return (void *)virtual;
    }

    // Look out for the next valid address
    virtual = find_next_free_region(p_size);
    if (!virtual)
    {
        return NULL;
    }

    if (!paging_map_region(p_address, virtual, p_size))
    {
        return NULL;
    }

    return (void *)virtual;
}


void paging_free_region(uint32_t p_virtual, uint32_t p_size)
{
    int directory_count = _get_directory_count(p_virtual, p_size);

    for (int i = 0; i < directory_count; i++)
    {
        uint16_t page_index = (p_virtual & 0xffc00000) >> 22;
        struct PageTable *table = (struct PageTable *)(page_directory[page_index + i] & 0xfffff000);
        page_directory[page_index + i] &= 2;            // Clear all bits except bit 1 (r/w bit)

        uint16_t table_start = 0;
        uint16_t table_end = 1024;
        if (i == 0)
        {
            table_start = (p_virtual & 0x003ff000) >> 12;
        }
        if (i == directory_count - 1)
        {
            table_end = ceil(((p_virtual + p_size - 1) % 0x400000), 4096);
            table_end = (table_end > 1024) ? 1024 : table_end;
        }

        // Can free entire page table
        bool free_pt = (table_start == 0 && table_end == 1024);

        for (int j = table_start; j < table_end; j++)
        {
            table->entry[j] = 0;
        }

        // Check if lower regions are empty as well
        bool lower_empty = false;
        if (table_start > 0)
        {
            for (int i = 0; i < table_start; i++)
            {
                if (table->entry[i] != 0) {
                    lower_empty = false;
                    break;
                }
            }

            lower_empty = true;
        }

        // Check if upper regions are empty as well
        bool upper_empty = false;
        if (table_end < 1024)
        {
            for (int i = table_end; i < 1024; i++)
            {
                if (table->entry[i] != 0) {
                    upper_empty = false;
                    break;
                }
            }

            upper_empty = true;
        }

        // If both lower and upper are empty, free the table.
        if (lower_empty && upper_empty)
        {
            free_pt = true;
        }

        if (free_pt) {
            _pt_free(table);
        }
    }
}


uint32_t virtual_to_physical(uint32_t p_virtual)
{
    uint16_t dir = (p_virtual & 0xffc00000) >> 22;
    struct PageTable *pt = (struct PageTable *)(page_directory[dir] & 0xfffff000);
    if (!pt || !(page_directory[dir] & 1))
    {
        return 0;
    }
    
    uint16_t idx = (p_virtual & 0x003ff000) >> 12;
    uint32_t ret = pt->entry[idx] & 0xfffff000;
    if (!ret || !(pt->entry[idx] & 1))
    {
        return 0;
    }

    return ret + (p_virtual & 0x00000fff);
}

uint32_t physical_to_virtual(uint32_t p_address)
{
    // We know that memory between 0x0 and 0x10000 is identity-mapped (for DMA and BIOS reasons) so we can return that address quickly.
    if (p_address < 0x10000)
    {
        return p_address;
    }

    // Address is in kernel-mapped memory
    if (p_address >= KERNEL_PHYSICAL_ADDRESS && p_address <= virtual_to_physical((uint32_t)&__end))
    {
        // Kernel offset is the same, bits are overriden.
        return p_address | KERNEL_VIRTUAL_ADDRESS;
    }

    // Page offset bits
    uint32_t offset = p_address & 0x00000fff;
    // AND the result so we align to the page memory
    p_address &= 0xfffff000;

    // Check allocated pages first (most uses of page lookups come from here)
    for (int i = 0; i < PAGE_TABLE_COUNT; i++)
    {
        // Found an allocated table
        if (config->page_info[i] & 1)
        {
            struct PageTable *pt = &allocated_tables[i];

            if (!pt)
            {
                return 0;
            }

            for (int j = 0; j < 1024; j++)
            {
                if (pt->entry[j] == 0) continue;

                // Found page
                if (p_address == pt->entry[j])
                {
                    return (i << 22) | (j << 12) | offset;
                }
            }
        }
    }

    // Unmapped physical address
    return 0;
}

bool is_valid_address(void *p_virtual)
{
    return virtual_to_physical((uint32_t)p_virtual) != 0;
}

bool is_valid_range(uint32_t p_start, uint32_t p_end)
{
    // Start and end may need to be mapped
    if (virtual_to_physical(p_start) == 0 || virtual_to_physical(p_end) == 0)
    {
        return false;
    }

    int range_size = p_end - p_start;
    while (range_size > 0) {
        if (virtual_to_physical(p_end - range_size) == 0)
        {
            return false;
        }

        range_size -= 4096;
    }

    return true;
}
