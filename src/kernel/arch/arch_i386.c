#include "arch.h"

#include <arch/io.h>

#include <cpuid.h>

#ifdef __I386__

#include "i386/gdt.h"
#include "i386/tss.h"
#include "i386/isr.h"
#include "i386/idt.h"
#include "i386/v86/v86_monitor.h"

static bool a_is_virtual = false;

void i386_check_is_virtual() {
    // Check CPUID first
    unsigned int regs[4];
    __get_cpuid(1, regs, &regs[1], &regs[2], &regs[3]);
    // Has hypervisor bit set
    if (regs[3] & 1 << 31) {
        a_is_virtual = true;
    }

    // Now check for bochs VBE
    if (!a_is_virtual) {
        outw(0x01ce, 0);
        if (inw(0x1cf) > 0xb0c0) {
            a_is_virtual = true;
        }
    }
}

bool arch_init() {
    i386_gdt_initialize();
    i386_tss_initialize();
    i386_isr_initialize();
    i386_idt_initialize();
    i386_check_is_virtual();
    return true;
}

bool arch_is_virtualized() {
    return a_is_virtual;
}

bool arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc) {
    return v86_run_task(p_start, p_end, p_args, p_argc);
}

bool arch_register_isr_handler(uint8_t p_isr, InterruptHandler p_handle) {
    return i386_isr_register_handler(p_isr, p_handle);
}

void arch_send_eoi(uint8_t p_irq) {
    if (p_irq >= 8) {
        outb(0xa0, 0x20);
    }

    outb(0x20, 0x20);
}

#endif