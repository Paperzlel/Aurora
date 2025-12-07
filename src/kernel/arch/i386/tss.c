#include "tss.h"

#include "gdt.h"

struct TSS a_tss_global =
{
    .ss0 = KERNEL32_DATA_SEGMENT,
    .es = KERNEL32_DATA_SEGMENT,
    .cs = KERNEL32_CODE_SEGMENT,
    .ds = KERNEL32_DATA_SEGMENT,
    .fs = KERNEL32_DATA_SEGMENT,
    .gs = KERNEL32_DATA_SEGMENT,
    .esp0 = 0xfff0,             // Manually overridden
    .iobp = sizeof(struct TSS) - 4
};

struct TSS_Descriptor a_tss_desc =
{
    sizeof(a_tss_global) - 4,
    &a_tss_global
};

void __attribute__((cdecl)) i386_tss_load(uint16_t p_offset);

struct TSS_Descriptor *i386_tss_get_descriptor()
{
    return &a_tss_desc;
}

void i386_tss_initialize()
{
    i386_tss_load(0x28);
}

void i386_tss_set_esp(uint32_t p_value)
{
    a_tss_global.esp0 = p_value;
}

void i386_tss_set_eip(uint32_t p_value)
{
    a_tss_global.eip = p_value;
}