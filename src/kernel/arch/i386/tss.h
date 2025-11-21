#pragma once

#include <stdint.h>

typedef struct {
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint16_t reserved;
    uint16_t iobp;
    uint32_t ssp;
} __attribute__((packed)) TSS;

typedef struct {
    uint32_t size;
    TSS *address;
} __attribute__((packed)) TSS_Descriptor;

TSS_Descriptor *i386_tss_get_descriptor();

void i386_tss_initialize();

void i386_tss_set_esp(uint32_t p_value);
void i386_tss_set_eip(uint32_t p_value);