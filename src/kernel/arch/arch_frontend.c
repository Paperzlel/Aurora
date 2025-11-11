#include "arch_frontend.h"

#include <cpuid.h>

#include "i686/v86/v86_monitor.h"
#include "i686/io.h"
#include "i686/isr.h"
#include "i686/i686_manager.h"

/**
 * Supported architectures are everything above the i486. I literally couldn't get bochs to the bootloader with an i386 CPU.
 * The bootloader AND kernel need to be compiled for that architecture, or they won't work.
 * TODO: Add an error in the bootloader that lets the user know they are using a too-low CPU architecture.
 */

typedef struct {
    void (*initialize)();
    void (*finalize)();
    bool (*register_interrupt_handler)(int, bool (*)(Registers *));
    void (*unregister_interrupt_handler)(int);
    bool (*run_task)(void *, void *, uint8_t *, int);
    void (*get_msr)(uint32_t, uint32_t *, uint32_t *);
    void (*set_msr)(uint32_t, uint32_t, uint32_t);
    uint8_t (*inb)(uint16_t);
    uint16_t (*inw)(uint16_t);
    void (*outb)(uint16_t, uint8_t);
    void (*outw)(uint16_t, uint16_t);
} ArchState;

static ArchState a_arch_state;

typedef struct {
    int eax;
    int ebx;
    int ecx;
    int edx;
} CPUID_Regs;

CPUID_Regs a_cpuid_regs;

int call_cpuid(int p_value) {
    return __get_cpuid(p_value, &a_cpuid_regs.eax, &a_cpuid_regs.ebx, &a_cpuid_regs.ecx, &a_cpuid_regs.edx);
}

uint16_t get_processor_family() {
    int val = call_cpuid(1);
    return (a_cpuid_regs.eax & 0xf00) >> 8;
}

bool arch_init() {
    // Check CPUID family
    uint16_t family = get_processor_family();
    if (family < 4) {
        // Somehow less than 0x04 which should be impossible, but check anyway.
        return false;
    }

    switch (family) {
        case 0x06: {    // i686
            a_arch_state.initialize = i686_initialize;
            a_arch_state.finalize = i686_finalize;
            a_arch_state.register_interrupt_handler = i686_isr_register_handler;
            a_arch_state.unregister_interrupt_handler = i686_isr_unregister_handler;
            a_arch_state.run_task = v86_run_task;
            a_arch_state.get_msr = i686_get_msr;
            a_arch_state.set_msr = i686_set_msr;
            a_arch_state.inb = i686_inb;
            a_arch_state.inw = i686_inw;
            a_arch_state.outb = i686_outb;
            a_arch_state.outw = i686_outw;
        } break;
        default: {      // Presumed i686
            a_arch_state.initialize = i686_initialize;
            a_arch_state.finalize = i686_finalize;
            a_arch_state.register_interrupt_handler = i686_isr_register_handler;
            a_arch_state.unregister_interrupt_handler = i686_isr_unregister_handler;
            a_arch_state.run_task = v86_run_task;
            a_arch_state.get_msr = i686_get_msr;
            a_arch_state.set_msr = i686_set_msr;
            a_arch_state.inb = i686_inb;
            a_arch_state.inw = i686_inw;
            a_arch_state.outb = i686_outb;
            a_arch_state.outw = i686_outw;
        } break;
    }

    a_arch_state.initialize();
    return true;
}

bool arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc) {
    return a_arch_state.run_task(p_start, p_end, p_args, p_argc);
}

uint8_t arch_io_inb(uint16_t p_port) {
    return a_arch_state.inb(p_port);
}

uint16_t arch_io_inw(uint16_t p_port) {
    return a_arch_state.inw(p_port);
}

void arch_io_outb(uint16_t p_port, uint8_t p_value) {
    a_arch_state.outb(p_port, p_value);
}

void arch_io_outw(uint16_t p_port, uint16_t p_value) {
    a_arch_state.outw(p_port, p_value);
}

void arch_set_msr(uint32_t p_msr, uint32_t p_lower, uint32_t p_higher) {
    a_arch_state.set_msr(p_msr, p_lower, p_higher);
}

void arch_get_msr(uint32_t p_msr, uint32_t *p_lower, uint32_t *p_higher) {
    a_arch_state.get_msr(p_msr, p_lower, p_higher);
}

bool arch_is_virtualized() {
    arch_io_outw(0x01ce, 0);
    return arch_io_inw(0x01cf) >= 0xb0c0;
}
