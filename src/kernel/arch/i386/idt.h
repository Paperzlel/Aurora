#pragma once

#include <stdint.h>

typedef enum {
    IDT_TASK_GATE = 0x5,
    IDT_INTERRUPT16 = 0x6,
    IDT_TRAP16 = 0x7,
    IDT_INTERRUPT32 = 0xe,
    IDT_TRAP32 = 0xf,

    IDT_RING0 = 0 << 5,
    IDT_RING1 = 1 << 5,
    IDT_RING2 = 2 << 5,
    IDT_RING3 = 3 << 5,

    IDT_PRESENT = 0x80
} IDT_Flags;

/**
 * @brief Changes the present flag for the given ISR so that it can be used by the rest of our code.
 * @param p_interrupt The interrupt to enable
 */
void i386_idt_enable_isr(int p_interrupt);

/**
 * @brief Changes the present flag for the given ISR so that it is no longer checked when said interrupt is called, usually to create a manual handler.
 * Do not disable if you don't want to handle it manually, as the ISR call will otherwise go to the BIOS, and may cause a reboot.
 * @param p_interrupt The interrupt to disable
 */
void i386_idt_disable_isr(int p_interrupt);

/**
 * @brief Registers the given ISR to call to the given function pointer (passed as void *) whenever the ISR is needed. By default you should need to do this as
 * all 256 ISRs are setup on launch of the kernel.
 * @param p_interrupt The ISR to attach the function pointer to
 * @param p_address The function pointer to use
 * @param p_segment_descriptor The segment descriptor this should run in (same as kernel, 0x08)
 * @param p_flags Any flags the ISR should need, mainly running in ring 0 and being a 32-bit interrupt
 */
void i386_idt_register_isr(int p_interrupt, void *p_address, uint16_t p_segment_descriptor, uint8_t p_flags);

/**
 * @brief Initializes the interrupt subsystem by loading the configured IDT into the IDTR.
 */
void i386_idt_initialize();