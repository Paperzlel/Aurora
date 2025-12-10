#include <aurora/memory.h>
#include <aurora/memdefs.h>
#include <boot/bootstructs.h>

#include "paging.h"

#define AUR_MODULE "memory"
#include <aurora/debug.h>

#include <stdio.h>
#include <string.h>

// Macro to align a memory address to N number of bytes. m_bytes must be a power of 2.
#define ALIGN(m_addr, m_bytes) ((m_addr + (m_bytes - 1)) & ~(m_bytes - 1))

enum MemoryHeaderFlags
{
    BIT_AVAILABLE = 1 << 0,
    BIT_USED = 0,
    BIT_KERNEL = 1 << 1,
    BIT_USERSPACE = 0,
};

struct MemoryHeader
{
    struct MemoryHeader *next;
    struct MemoryHeader *prev;
    uint32_t size;
    uint32_t parent_flags;
};

struct HeapHeader
{
    struct HeapHeader *next;                // Pointer to the next heap in the list
    struct HeapHeader *prev;                // Pointer to the previous heap in the list
    struct MemoryHeader *list;              // Pointer to the list of allocations the thread manages
    uint32_t size;                          // The number of bytes the heap is allowed to handle. A very large allocation needs to specify this.
    uint32_t available_space;               // Number of bytes (excluding headers) that the heap has available.
    uint32_t allocations;                   // Number of allocated blocks of memory on the heap.
    uint32_t flags;                         // Identifiers about the information on the stack.
    uint32_t reserved;                      // Currently unused.
};

struct MemoryConfig
{
    uint64_t available_memory;
    uint64_t reserved_memory;
    uint32_t physical_mem_start;
};

STATIC_ASSERT(sizeof(struct HeapHeader) % 16 == 0, "HeapHeader must be aligned to a 16-byte boundary.");
STATIC_ASSERT(sizeof(struct MemoryHeader) == 16, "MemoryHeader must be 16 bytes in size.");

/* HEAP FUNCTIONS */

static struct HeapHeader *heap_root = NULL;
static struct MemoryConfig memcfg = { 0 };

static struct HeapHeader *_aheap_alloc(size_t p_mibibyte_count, size_t p_address);

/* MEMORY MANAGEMENT */

static void _a_mmap_create(struct MemoryMap *map);


bool initialize_memory(struct MemoryMap *p_map, uint32_t p_kernel_size)
{
    // Already mapped memory, return
    if (heap_root && is_valid_address(heap_root) && heap_root->size != 0)
    {
        return true;
    }
    
    memcfg.physical_mem_start = KERNEL_PHYSICAL_ADDRESS + p_kernel_size;

    // Setup paging first
    paging_initialize(&memcfg.physical_mem_start);

    // Allocate root heap
    struct HeapHeader *heap = _aheap_alloc(0x04, 0x00);

    if (!heap)
    {
        return false;
    }

    // Create a physical memory map for the system.
    _a_mmap_create(p_map);
    memcfg.available_memory -= p_kernel_size;
    memcfg.reserved_memory += p_kernel_size;

    LOG_INFO("Total memory available: %llu bytes (%llu MiB)", memcfg.available_memory, memcfg.available_memory / MIBIBYTES_TO_BYTES);
    LOG_INFO("Total reserved memory: %llu bytes (%llu MiB)", memcfg.reserved_memory, memcfg.reserved_memory / MIBIBYTES_TO_BYTES);
    return true;
}


void *kalloc(uint32_t p_size)
{
    // Align to the next 16 byte interval
    p_size = ALIGN(p_size, 0x10);

    struct HeapHeader *heap = heap_root;
    while (heap)
    {
        if (heap->available_space > p_size + sizeof(struct MemoryHeader)) break;
        heap = heap->next;
    }

    if (!heap)
    {
        heap = _aheap_alloc(0x04, 0x00);
        if (!heap)
        {
            LOG_ERROR("Failed to allocate a new heap.");
            return NULL;
        }
    }

    struct MemoryHeader *header = heap->list;
    while (header->next)
    {
        if (header->parent_flags & BIT_AVAILABLE && header->size >= p_size) break;
        header = header->next;
    }

    
    if (!header->next)
    {
        // Make a new header at the end of memory
        struct MemoryHeader new_header = { 0 };
        new_header.size = p_size;
        new_header.parent_flags = ((uint32_t)heap & 0xffffff00) | BIT_KERNEL;
        new_header.prev = header;
        struct MemoryHeader *dest = (struct MemoryHeader *)((void *)header + sizeof(struct MemoryHeader) + header->size);
        memcpy(dest, &new_header, sizeof(struct MemoryHeader));
        header->next = dest;
        heap->available_space -= p_size + sizeof(struct MemoryHeader);
        heap->allocations++;
        return (void *)dest + sizeof(struct MemoryHeader);
    }

    // Clear available bit 
    header->parent_flags &= ~BIT_AVAILABLE;
    heap->available_space -= header->size;
    return (void *)header + sizeof(struct MemoryHeader);
}


void kfree(void *p_mem)
{
    struct MemoryHeader *header = p_mem - sizeof(struct MemoryHeader);
    struct HeapHeader *heap = (struct HeapHeader *)(header->parent_flags & 0xffffff00);

    heap->available_space += header->size;
    header->parent_flags |= BIT_AVAILABLE;
}


bool kmap_range(uint32_t p_physical, uint32_t p_virtual, uint32_t p_size)
{
    //TODO: Add more here
    return paging_map_region(p_physical, p_virtual, p_size);
}


bool is_4kib_aligned(void *p_address)
{
    return ((uint32_t)p_address & 0xfffff000) == (uint32_t)p_address;
}


/* MEMORY MAP */

struct AuMemoryRegion
{
    uint64_t base_address;              // The starting physical address of the region
    uint64_t length_blocked;            // The length of the region, assuming it is aligned to a power of 2 greater than 1.
};

// Static array of memory regions
struct AuMemoryInfo
{
    size_t region_count;
    struct AuMemoryRegion *regions;
};

static struct AuMemoryInfo *a_mmap_info = NULL;
static struct MemoryHeader *a_mmap_header = NULL;


#define AUR_MMAP_NEW_BLOCK(m_name, m_base, m_size, m_usable)                                                                    \
    struct AuMemoryRegion *a##m_name = &a_mmap_info->regions[a_mmap_info->region_count];                                        \
    a##m_name->base_address = m_base;                                                                                           \
    a##m_name->length_blocked = (m_size) | (m_usable);                                                                          \
    a_mmap_info->region_count++;                                                                                                \
    a_mmap_header->size += sizeof(struct AuMemoryRegion);                                                                       \
    ((struct HeapHeader *)(a_mmap_header->parent_flags & 0xffffff00))->available_space -= sizeof(struct AuMemoryRegion)

#define AUR_MMAP_DEFAULT() AUR_MMAP_NEW_BLOCK(, mr.base_address, mr.length, mr.type)


static void _a_mmap_create(struct MemoryMap *map)
{
    // We manage our blocklist here by raw-allocating with no checks to if the memory is valid whatsoever.
    a_mmap_header = heap_root->list;
    a_mmap_header->parent_flags = (uint32_t)heap_root->list & 0xffffff00;
    a_mmap_header->parent_flags |= BIT_USED;
    heap_root->allocations++;

    a_mmap_info = (struct AuMemoryInfo *)((void *)heap_root->list + sizeof(struct MemoryHeader));
    a_mmap_info->regions = (struct AuMemoryRegion *)((void *)a_mmap_info + sizeof(struct AuMemoryInfo));
    // TODO: Notify heap of allocation changes
    a_mmap_header->size = sizeof(struct AuMemoryInfo);

    bool did_swap = false;
    for (int i = 0; i < map->region_count - 1; i++)
    {
        did_swap = false;
        for (int j = 0; j < map->region_count - i - 1; j++)
        {
            if (map->regions[j].base_address > map->regions[j + 1].base_address)
            {
                struct MemoryRegion tmp = map->regions[j];
                map->regions[j] = map->regions[j + 1];
                map->regions[j + 1] = tmp;
                did_swap = true;
            }
        }

        if (!did_swap) break;
    }

    for (int i = 0; i < map->region_count; i++)
    {
        struct MemoryRegion mr = map->regions[i];
        // Map regions if needed
        if ((mr.type == MEMORY_REGION_ACPI_NVS || (mr.type == MEMORY_REGION_RESERVED && !is_valid_address((void *)((uint32_t)mr.base_address)))) && 
            !paging_map_region(mr.base_address, mr.base_address, mr.length))
        {
            LOG_ERROR("Failed to map region %x (size %x)", mr.base_address, mr.length);
            continue;
        }

        // Set all non-usable types to 0 for simplicity
        if (mr.type != MEMORY_REGION_USABLE)
        {
            mr.type = 0;
        }

        struct AuMemoryRegion *prev = (a_mmap_info->region_count > 0) ? &a_mmap_info->regions[a_mmap_info->region_count - 1] : NULL;

        /**
         * Four conditions:
         * 1. Previous block is usable and this is usable
         * 2. Previous block is usable and this is blocked
         * 3. Previous block is blocked and this is usable
         * 4. Previous block is blocked and this is blocked
         */

        if (!prev)
        {
            // No previous allocations made, create a region
            AUR_MMAP_DEFAULT();
            continue;
        }

        // Regions overlap in some way
        if ((prev->base_address + (prev->length_blocked & ~1) >= mr.base_address))
        {
            uint64_t prev_max_address = prev->base_address + (prev->length_blocked & ~1);

            if ((prev->length_blocked & 1) == mr.type)
            {
                // Usable region
                // Max entirely overlaps the region
                if (prev_max_address > mr.base_address + mr.length) continue;
                // Max partially overlaps the region
                prev->length_blocked = (mr.base_address - prev->base_address + mr.length) | mr.type;
            }
            else
            {
                // Usable region
                // Bit for usability is the same, OR it in
                prev->length_blocked = (mr.base_address - prev->base_address) | (prev->length_blocked & 1);

                // use region count here because i != region_count
                struct AuMemoryRegion *current = &a_mmap_info->regions[a_mmap_info->region_count];
                AUR_MMAP_DEFAULT();

                // Max entirely overlaps the region
                if (prev_max_address > mr.base_address + mr.length)
                {
                    // Allocate next block up-front
                    uint64_t base = current->base_address + current->length_blocked;
                    AUR_MMAP_NEW_BLOCK(prev, base, prev_max_address - base, prev->length_blocked & 1);
                }
            }
            continue;
        }

        AUR_MMAP_DEFAULT();
    }

    // Properly align size and get the proper remaining space.
    heap_root->available_space -= ALIGN(a_mmap_header->size, 0x10) - a_mmap_header->size;
    a_mmap_header->size = ALIGN(a_mmap_header->size, 0x10);

    for (int i = 0; i < a_mmap_info->region_count; i++)
    {
        memcfg.available_memory += (a_mmap_info->regions[i].length_blocked & 1) ? (a_mmap_info->regions[i].length_blocked & ~1) : 0;
        memcfg.reserved_memory += (a_mmap_info->regions[i].length_blocked & 1) ? 0 : (a_mmap_info->regions[i].length_blocked & ~1);
    }
}

#undef AUR_MMAP_DEFAULT
#undef AUR_MMAP_NEW_BLOCK


static struct HeapHeader *_aheap_alloc(size_t p_mibibyte_count, size_t p_address)
{
    if (!p_mibibyte_count)
    {
        // Min heap size of 1 MiB
        p_mibibyte_count = 1;
    }
    p_mibibyte_count *= MIBIBYTES_TO_BYTES;

    struct HeapHeader *ret = NULL;

    if (!heap_root)
    {
        // Setup root header of 4MiB
        void *first_mem_region = paging_allocate_region(memcfg.physical_mem_start, p_mibibyte_count);
        if (!first_mem_region)
        {
            LOG_ERROR("Failed to allocate root heap properly.");
            return NULL;
        }

        heap_root = (struct HeapHeader *)first_mem_region;
        heap_root->next = NULL;
        heap_root->prev = NULL;
        ret = heap_root;
    }
    else
    {
        struct HeapHeader *mem = heap_root;
        while (mem->next)
        {
            mem = mem->next;
        }

        if (!p_address)
        {
            p_address = virtual_to_physical((uint32_t)mem) + mem->size;
        }

        struct HeapHeader *nhp = paging_allocate_region(p_address, p_mibibyte_count);
        if (!nhp)
        {
            LOG_ERROR("Failed to allocate new heap in memory.");
            return NULL;
        }

        mem->next = nhp;
        nhp->prev = mem;
        nhp->next = NULL;
        ret = nhp;
    }

    ret->allocations = 0;
    ret->list = (struct MemoryHeader *)((void *)ret + sizeof(struct HeapHeader));
    ret->size = p_mibibyte_count;
    ret->available_space = ret->size - sizeof(struct HeapHeader) - sizeof(struct MemoryHeader);

    ret->list->next = NULL;
    ret->list->prev = NULL;
    ret->list->size = 0;
    ret->list->parent_flags = ((uint32_t)ret & 0xffffff00) | BIT_AVAILABLE | BIT_KERNEL;
    return ret;
}
