#include "gdt.h"
#include "tss.h"

#include <stdint.h>

// Struct that describes any entry in the GDT. Some parts contain more than one item
typedef struct {
    uint16_t limit;                 // First 16 bits of the limit (20 bits total)
    uint16_t base_low;              // First 16 bits of the base (32 bits total)
    uint8_t base_mid;               // Next 8 bits of the base
    uint8_t access;                 // Access flags, see GDT_AccessFlags
    uint8_t flags_limit;            // General flags + last 4 bits of limit
    uint8_t base_high;              // Last 8 bits of the base
} __attribute__((packed)) GDT_Entry;

// Struct that describes the overall data within the GDT.
typedef struct {
    uint16_t size;                  // Total size of the GDT - 1
    GDT_Entry *gdt_addr;            // Pointer to the GDT in memory
} __attribute__((packed)) GDT_Description;

// Enum for the access flag in the GDT.
typedef enum {
    GDT_ACCESSED = 1 << 0,              // Bit that is marked by the CPU when accessed
    
    GDT_READABLE = 1 << 1,              // Readable code segment.
    GDT_WRITABLE = 1 << 1,              // Writable data segment.

    GDT_DIR_UPWARDS = 0,                // Data selector grows upwards
    GDT_DIR_DOWNWARDS = 1 << 2,         // Data selector grows downwards
    
    GDT_DPL_ONLY = 0,                   // Code selector can only be executed from the ring in the DPL.
    GDT_DPL_EQU_OR_LOWER = 1 << 2,      // Code selector can be executed from an equal or lesser privilege level

    GDT_CODE_SEGMENT = 1 << 3,          // Defines a code segment.
    GDT_DATA_SEGMENT = 0,               // Defines a data segment.

    GDT_CODE_DATA_SEGMENT = 1 << 4,     // Defines a code/data segment. Must be set for the above to work.
    GDT_SYSTEM_SEGMENT = 0,             // Defines a system segment, i.e. a TSS.

    GDT_PRESENT = 1 << 7                // Present bit. Must be set for the segment to be valid.
} GDT_AccessFlags;

// Enum for different rings. Used for the DPL bit
typedef enum {
    GDT_RING0 = 0 << 5,     // Ring 0 / kernel
    GDT_RING1 = 1 << 5,     // Ring 1 / drivers
    GDT_RING2 = 2 << 5,     // Ring 2 / drivers
    GDT_RING3 = 3 << 5      // Ring 3 / userspace
} GDT_Rings;

// Enum for flags within the GDT.
typedef enum {
    GDT_64BIT = 1 << 5,             // Use a 64-bit register
    GDT_32BIT = 1 << 6,             // Use a 32-bit register (if both are unset, then the descriptor is 16-bit)
    GDT_16BIT = 0,                  // Use a 16-bit register

    GDT_GRANULARITY_4KIB = 1 << 7,      // Use 4-KiB blocks/page granularity 
    GDT_GRANULARITY_1B = 0              // Use 1-byte blocks/byte granularity
} GDT_Flags;

// Enum for the system segment descriptor flags
typedef enum {
    GDT_TYPE_TSS16_AV = 1 << 0,
    GDT_TYPE_LDT = 1 << 1,
    GDT_TYPE_TSS16_BU = 3,
    GDT_TYPE_TSS32_AV = 9,
    GDT_TYPE_TSS32_BU = 0xB,
} GDT_SystemFlags;

/**
 * @brief Macro that describes one entry within the GDT.
 * @param base The 32-bit base to use
 * @param limit The 20-bit limit to use
 * @param access The access flags
 * @param flags The general flags
 */
#define GDT_ENTRY(base, limit, access, flags)                   \
    (limit & 0xffff),                                           \
    (base & 0xffff),                                            \
    ((base >> 16) & 0xff),                                      \
    access,                                                     \
    (((limit >> 16) & 0xf) | (flags & 0xf0)),                   \
    ((base >> 24) & 0xff)

#define GDT_ADDENTRY(m_entry, m_base, m_limit, m_access, m_flags)                       \
    m_entry.limit = (m_limit & 0xffff);                                                 \
    m_entry.base_low = (m_base & 0xffff),                                               \
    m_entry.base_mid = ((m_base >> 16) & 0xff),                                         \
    m_entry.access = m_access,                                                          \
    m_entry.flags_limit = (((m_limit >> 16) & 0xf) | (m_flags & 0xf0)),                 \
    m_entry.base_high = ((m_base >> 24) & 0xff)

// Same segments as bootloader/stage2/entry.asm
GDT_Entry a_gdt_entries[6] = {
    GDT_ENTRY(0, 0, 0, 0),
    // Code segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_CODE_SEGMENT | GDT_CODE_DATA_SEGMENT | GDT_READABLE, GDT_GRANULARITY_4KIB | GDT_32BIT),

    // Data segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_DATA_SEGMENT | GDT_CODE_DATA_SEGMENT | GDT_WRITABLE , GDT_GRANULARITY_4KIB | GDT_32BIT),

    // 16-bit code segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_CODE_SEGMENT | GDT_CODE_DATA_SEGMENT | GDT_READABLE, GDT_GRANULARITY_1B | GDT_16BIT),

    // 16-bit data segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_DATA_SEGMENT | GDT_CODE_DATA_SEGMENT | GDT_WRITABLE, GDT_GRANULARITY_1B | GDT_16BIT),

    // 32-bit TSS (added manually)
};

// Segment selectors use the model index (15 - 3), table (2), privelidge level (1 - 0)
// Meaning 0x08 is our segment selector for CS, 0x10 for our DS
// The TSS will be after the 16-bit registers (values 0x18 and 0x20, to value 0x28)

GDT_Description a_desc = {
    sizeof(a_gdt_entries) - 1,
    a_gdt_entries
};

/**
 * @brief Loads the GDT into the GDTR from assembly.
 * @param p_gdt The GDT to use
 * @param p_code_segment The code segment to use
 * @param p_data_segment The data segment to use
 */
void __attribute__((cdecl)) i686_gdt_load(GDT_Description *p_gdt, uint8_t p_code_segment, uint8_t p_data_segment);

void i686_gdt_initialize() {

    TSS_Descriptor *p_desc = i686_tss_get_descriptor();

    GDT_ADDENTRY(a_gdt_entries[5], (uint32_t)p_desc->address, p_desc->size, 
        GDT_PRESENT | GDT_RING0 | GDT_SYSTEM_SEGMENT | GDT_TYPE_TSS32_AV, 
        GDT_GRANULARITY_4KIB | GDT_32BIT);

    i686_gdt_load(&a_desc, KERNEL32_CODE_SEGMENT, KERNEL32_DATA_SEGMENT);
}