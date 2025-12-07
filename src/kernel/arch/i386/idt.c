#include "idt.h"

// Struct that represents an entry in the IDT.
struct __attribute__((packed)) IDT_Entry
{
    uint16_t offset_low;                // Low 16 bits of the linear address
    uint16_t segment;                   // Segment descriptor
    uint8_t reserved;                   // Reserved 8 bits, set to null
    uint8_t type_flags;                 // Flags
    uint16_t offset_high;               // High 16 bits of linear address
};

// Struct that represents the IDT description to pass to lidt.
struct __attribute__((packed)) IDT_Descriptor
{
    uint16_t size;              // Side of the IDT - 1
    struct IDT_Entry *offset;   // Offset of the IDT in memory
};

// List of all IDT entries
struct IDT_Entry a_idt_entries[256];

struct IDT_Descriptor a_idt =
{
    sizeof(a_idt_entries) - 1,
    a_idt_entries
};

/**
 * @brief Loads the IDT from ASM.
 * @param p_desc The IDT descriptor
 */
void __attribute__((cdecl)) i386_idt_load(struct IDT_Descriptor *p_desc);

void i386_idt_enable_isr(int p_interrupt)
{
    a_idt_entries[p_interrupt].type_flags |= IDT_PRESENT;
}

void i386_idt_disable_isr(int p_interrupt)
{
    a_idt_entries[p_interrupt].type_flags &= 0x7f;  // IDT_PRESENT - 1
}


void i386_idt_register_isr(int p_interrupt, void *p_address, uint16_t p_segment_descriptor, uint8_t p_flags)
{
    a_idt_entries[p_interrupt].offset_low = ((uint32_t)p_address) & 0xffff;
    a_idt_entries[p_interrupt].segment = p_segment_descriptor;
    a_idt_entries[p_interrupt].reserved = 0;
    a_idt_entries[p_interrupt].type_flags = p_flags;
    a_idt_entries[p_interrupt].offset_high = ((uint32_t)p_address >> 16) & 0xffff;
}

void i386_idt_initialize()
{
    i386_idt_load(&a_idt);
}
