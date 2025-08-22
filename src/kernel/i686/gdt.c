#include "gdt.h"

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

// Enum for the acces flag in the GDT.
typedef enum {
    GDT_ACCESSED = 1 << 0,              // Bit that is marked by the CPU when accessed
    GDT_RW = 1 << 1,                    // Readable/Writable bit. Code segments get read access, data segments get write access
    GDT_DIRECTION = 1 << 2,             // Direction bit. Data selectors group up if clear, and code selectors can only execute from this ring if clear. 
    GDT_EXECUTABLE = 1 << 3,            // Executable bit. If clear, data segment. If set, code segment
    GDT_DESCRIPTOR_TYPE = 1 << 4,       // Descriptor type. If clear, system segment. If set, code/data segment
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
    GDT_LONG_MODE = 1 << 5,         // Use a 64-bit register
    GDT_32BIT = 1 << 6,             // Use a 32-bit register (if both are unset, then the descriptor is 16-bit)
    GDT_GRANULARITY = 1 << 7        // Use 4-KiB page granularity (max of 4 GiB of RAM)
} GDT_Flags;

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


// Same segments as bootloader/stage2/entry.asm
GDT_Entry a_gdt_entries[] = {
    GDT_ENTRY(0, 0, 0, 0),
    // Code segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_DESCRIPTOR_TYPE | GDT_EXECUTABLE | GDT_RW, GDT_GRANULARITY | GDT_32BIT),

    // Data segment
    GDT_ENTRY(0, 0xfffff, GDT_PRESENT | GDT_RING0 | GDT_DESCRIPTOR_TYPE | GDT_RW, GDT_GRANULARITY | GDT_32BIT)
};

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
    i686_gdt_load(&a_desc, GDT_CODE_SEGMENT, GDT_DATA_SEGMENT);
}