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
#define ALIGN32(m_addr) ALIGN(m_addr, 0x20)

enum MemoryFlags
{
    BIT_AVAILABLE   = 1 << 0,
    BIT_USED        = 0 << 0,
    BIT_KERNEL      = 1 << 1,
    BIT_USERSPACE   = 0 << 1,
    BIT_HEADERS     = 1 << 2,
    BIT_FREE_MEM    = 0 << 2,
};

struct MemoryHeader
{
    struct MemoryHeader *next;              // Points to the next header in memory
    uint32_t virt_address;                  // Virtual address of the memory itself (where it is paged to)
    uint32_t size;                          // Number of bytes allocated to the header (32-byte aligned)
    uint32_t parent_flags;                  // Pointer to the parent heap + 4 bits worth of flags
};

struct HeapHeader
{
    struct HeapHeader *next;                // Pointer to the next heap in the list
    struct HeapHeader *prev;                // Pointer to the previous heap in the list
    struct MemoryHeader *list;              // Pointer to the list of allocations the thread manages
    uint32_t size;                          // The number of bytes the heap is allowed to handle.
    uint32_t available_space;               // Number of bytes (excluding headers) that the heap has available.
    uint32_t allocations;                   // Number of allocated blocks of memory on the heap.
    uint32_t flags;                         // Identifiers about the information on the stack.
    uint32_t virt_address;                  // Virtual address for the heap
};

struct MemoryConfig
{
    uint64_t available_memory;
    uint64_t reserved_memory;
    uint32_t physical_mem_start;
    uint32_t next_free_physical_address;
};

STATIC_ASSERT(sizeof(struct HeapHeader) % 16 == 0, "HeapHeader must be aligned to a 16-byte boundary.");
STATIC_ASSERT(sizeof(struct MemoryHeader) == 16, "MemoryHeader must be 16 bytes in size.");

/* HEAP FUNCTIONS */

static struct HeapHeader *heap_root = NULL;
static struct MemoryConfig memcfg = { 0 };

static struct HeapHeader *_a_heap_alloc(size_t p_mibibyte_count, size_t p_address);
static struct MemoryHeader *_a_header_alloc(size_t p_size);

/* MEMORY MANAGEMENT */

static void _a_mmap_create(struct MemoryMap *map);
static bool _a_mmap_is_valid_physical_address(uint32_t p_address);


bool initialize_memory(struct MemoryMap *p_map, uint32_t p_kernel_size)
{
    // Already mapped memory, return
    if (heap_root && is_valid_address(heap_root) && heap_root->size != 0)
    {
        return true;
    }
    
    memcfg.physical_mem_start = KERNEL_PHYSICAL_ADDRESS + p_kernel_size;
    memcfg.next_free_physical_address = memcfg.physical_mem_start;

    // Setup paging first
    paging_initialize(&memcfg.physical_mem_start);

    // Allocate root heap (1 MiB, for heaps themselves)
    struct HeapHeader *heap = _a_heap_alloc(0x01, 0x00);

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
    struct HeapHeader *heap = heap_root;
    while (heap)
    {
        if (heap->available_space > p_size && !(heap->flags & BIT_HEADERS)) break;
        heap = heap->next;
    }

    if (!heap)
    {
        heap = _a_heap_alloc(0x04, 0x00);
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
        header->next = _a_header_alloc(p_size);
        if (!header->next) return NULL;

        header = header->next;
    }

    // Clear available bit 
    header->parent_flags &= ~BIT_AVAILABLE;
    heap->available_space -= ALIGN32(p_size);
    heap->allocations++;
    return (void *)header->virt_address;
}


void kfree(void *p_mem)
{
    struct HeapHeader *header = heap_root;
    while (header)
    {
        if (!(header->flags & BIT_HEADERS) && header->virt_address < (uint32_t)p_mem && header->virt_address + header->size > (uint32_t)p_mem) break;
        header = header->next;
    }

    // Trawl allocation list
    struct MemoryHeader *h = header->list;
    while (h)
    {
        if (h->virt_address == (uint32_t)p_mem) break;
        h = h->next;
    }

    if (h->parent_flags & BIT_AVAILABLE)
    {
        LOG_ERROR("Double free attempted.");
        return;
    }

    // Grow available memory if multiple headers are free
    h->parent_flags |= BIT_AVAILABLE;
    struct MemoryHeader *n = h->next;
    header->allocations--;
    header->available_space += ALIGN32(h->size);
    // NOTE: Current implementation only works forwards, backwards sorting may be needed.
    while (n)
    {
        if (!(n->parent_flags & BIT_AVAILABLE)) break;
        h->size += n->size;
        n = n->next;
    }
}


void *krealloc(void *ptr, size_t p_size)
{
    if (!ptr)
    {
        return kalloc(p_size);
    }

    struct HeapHeader *header = heap_root;
    while (header)
    {
        if (!(header->flags & BIT_HEADERS) && header->virt_address < (uint32_t)ptr && header->virt_address + header->size > (uint32_t)ptr) break;
        header = header->next;
    }

    // Trawl allocation list
    struct MemoryHeader *h = header->list;
    while (h)
    {
        if (h->virt_address == (uint32_t)ptr) break;
        h = h->next;
    }

    // NOTE: Since realloc() on a freed bit of memory is UB, we'll give the info a pass for the kernel, but should be warned about in any other context.

    // Different block size, do something
    if (ALIGN32(h->size) != ALIGN32(p_size))
    {
        // Memory allocations are always linear, so checking this way should always be fine
        if (h->virt_address + p_size > h->next->virt_address)
        {
            h->parent_flags |= BIT_AVAILABLE;
            header->allocations--;
            void *ret = kalloc(p_size);
            memcpy(ret, (void *)h->virt_address, h->size);
            return ret;
        }
        else
        {
            // Subtract new size from available memory
            if (ALIGN32(h->size) > ALIGN32(p_size))
            {
                header->available_space += (ALIGN32(h->size) - ALIGN32(p_size));
            }
            else
            {
                header->available_space -= (ALIGN32(p_size) - ALIGN32(h->size));
            }
        }
    }

    // Assign new size
    h->size = p_size;
    return ptr;
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
    a_mmap_header->size += sizeof(struct AuMemoryRegion)

#define AUR_MMAP_DEFAULT() AUR_MMAP_NEW_BLOCK(, mr.base_address, mr.length, mr.type)


static void _a_mmap_create(struct MemoryMap *map)
{
    // We manage our blocklist here by raw-allocating with no checks to if the memory is valid whatsoever.
    // Still need a header though, so we get one here.
    a_mmap_header = _a_header_alloc(0x20);
    a_mmap_header->size = 0;            // Zero size to fix allocation issues
    a_mmap_header->parent_flags &= ~BIT_AVAILABLE;
    ((struct HeapHeader *)(a_mmap_header->parent_flags & 0xfffffff0))->allocations++;

    // Allocate a memory region
    
    a_mmap_info = (struct AuMemoryInfo *)((void *)a_mmap_header->virt_address);
    a_mmap_info->regions = (struct AuMemoryRegion *)(a_mmap_info + sizeof(struct AuMemoryInfo));
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

    // Assign the properly aligned space.
    ((struct HeapHeader *)(a_mmap_header->parent_flags & 0xfffffff0))->available_space -= ALIGN(a_mmap_header->size, 0x20);

    for (int i = 0; i < a_mmap_info->region_count; i++)
    {
        memcfg.available_memory += (a_mmap_info->regions[i].length_blocked & 1) ? (a_mmap_info->regions[i].length_blocked & ~1) : 0;
        memcfg.reserved_memory += (a_mmap_info->regions[i].length_blocked & 1) ? 0 : (a_mmap_info->regions[i].length_blocked & ~1);
    }
}

#undef AUR_MMAP_DEFAULT
#undef AUR_MMAP_NEW_BLOCK

/**
 * @brief Checks to see if a given physical address lives within valid memory. 
 * @param p_address The address to check
 * @return `true` if yes, `false` if not.
 */
static bool _a_mmap_is_valid_physical_address(uint32_t p_address)
{
    // Just assume that all address space pre-allocating is valid
    if (!a_mmap_info) return true;

    for (int i = 0; i < a_mmap_info->region_count; i++)
    {
        if ((a_mmap_info->regions[i].base_address + (a_mmap_info->regions[i].length_blocked & ~1)) > p_address && (a_mmap_info->regions[i].length_blocked & 1))
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief Gets the first available memory from the next available region, if it exists.
 * @param p_from 
 * @return 
 */
static uint32_t _a_mmap_find_next_physical_address(uint32_t p_from)
{
    int region = 0;
    for (int i = 0; i < a_mmap_info->region_count; i++)
    {
        if ((a_mmap_info->regions[i].base_address + (a_mmap_info->regions[i].length_blocked & ~1)) > p_from)
        {
            region = i;
            continue;
        }

        if (region && (a_mmap_info->regions[i].length_blocked & 1))
        {
            return (uint32_t)a_mmap_info->regions[i].base_address;
        }
    }

    return 0;
}


/* HEAPS/HEADERS */


/**
 * @brief "Reserves" N bytes of memory on a header-based heap for another header to be located at the next address slot. Subtracts
 * the needed values for available space and increments the allocation count.
 * @param p_size The number of bytes needed to be reserved.
 * @return The pointer to the now-available memory.
 */
static void *_a_heap_reserve_memory(size_t p_size)
{
    struct HeapHeader *header = heap_root;
    while (header)
    {
        if (heap_root->flags & BIT_HEADERS && heap_root->available_space > p_size) break;
        header = header->next;
    }

    if (!header) return NULL;

    void *next = (void *)header + header->size - header->available_space;
    header->allocations++;
    header->available_space -= ALIGN32(p_size);
    return next;
}


static struct HeapHeader *_a_heap_alloc(size_t p_mibibyte_count, size_t p_address)
{
    if (!p_mibibyte_count)
    {
        // Min heap size of 1 MiB
        p_mibibyte_count = 1;
    }
    p_mibibyte_count *= MIBIBYTES_TO_BYTES;

    if (!heap_root)
    {
        // Setup root header which hold all the pages for memory allocations
        heap_root = (struct HeapHeader *)paging_allocate_region(memcfg.physical_mem_start, p_mibibyte_count);
        if (!heap_root)
        {
            LOG_ERROR("Failed to allocate root heap properly.");
            return NULL;
        }

        heap_root->next = NULL;
        heap_root->prev = NULL;
        heap_root->list = NULL;
        heap_root->size = p_mibibyte_count;
        heap_root->available_space = p_mibibyte_count - sizeof(struct HeapHeader);
        heap_root->allocations = 1;
        heap_root->flags = BIT_KERNEL | BIT_AVAILABLE | BIT_HEADERS;
        heap_root->virt_address = (uint32_t)heap_root;
        return heap_root;
    }

    struct HeapHeader *mem = heap_root;
    while (mem->next)
    {
        mem = mem->next;
    }

    if (!p_address)
    {
        p_address = memcfg.next_free_physical_address;
    }

    if (!_a_mmap_is_valid_physical_address(p_address))
    {
        p_address = _a_mmap_find_next_physical_address(p_address);
        if (!p_address)
        {
            LOG_ERROR("Ran out of physical memory.");
            return NULL;
        }
    }

    void *nhp = paging_allocate_region(p_address, p_mibibyte_count);
    if (!nhp)
    {
        LOG_ERROR("Failed to allocate new heap in memory.");
        return NULL;
    }

    // Change free phys address TODO: Check if the new address is valid.
    memcfg.next_free_physical_address += p_mibibyte_count;

    struct HeapHeader h = { 0 };
    h.prev = mem;
    h.next = NULL;
    h.list = NULL;
    h.size = p_mibibyte_count;
    h.available_space = p_mibibyte_count;
    h.allocations = 0;
    h.flags = BIT_KERNEL | BIT_AVAILABLE | BIT_FREE_MEM;        // NOTE: Need to say when it's a header
    h.virt_address = (uint32_t)nhp;

    struct HeapHeader *ptr = (struct HeapHeader *)_a_heap_reserve_memory(sizeof(struct HeapHeader));

    if (!memcpy(ptr, &h, sizeof(struct HeapHeader)))
    {
        LOG_ERROR("Failed to copy the heap to memory.");
        return NULL;
    }

    ptr->prev->next = ptr;
    return ptr;
}


static struct MemoryHeader *_a_header_alloc(size_t p_size)
{
    struct HeapHeader *heap = heap_root;
    while (heap)
    {
        if (!(heap->flags & BIT_HEADERS) && heap->available_space > p_size) break;
        heap = heap->next;
    }
    
    // Allocate heap if needed
    if (!heap)
    {
        heap = _a_heap_alloc(0x04, 0);
        if (!heap)
        {
            LOG_ERROR("Failed to allocate heap.");
            return NULL;
        }
    }

    struct MemoryHeader *mem = _a_heap_reserve_memory(sizeof(struct MemoryHeader));
    if (!mem)
    {
        LOG_ERROR("Failed to reserve root memory header.");
        return NULL;
    }

    mem->next = NULL;
    mem->size = p_size;
    mem->virt_address = heap->virt_address + heap->size - heap->available_space;
    mem->parent_flags = ((uint32_t)heap & 0xfffffff0) | BIT_AVAILABLE | BIT_KERNEL;

    // Create first memory header, if non-existent
    if (!heap->list)
    {
        heap->list = mem;
    }
    else
    {
        struct MemoryHeader *end = heap->list;
        while (end->next)
        {
            end = end->next;
        }

        end->next = mem;
    }

    return mem;
}
