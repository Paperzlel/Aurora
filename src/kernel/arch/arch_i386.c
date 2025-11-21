#include "arch.h"

#include "io.h"

#ifdef __I386__

#include "i386/gdt.h"
#include "i386/tss.h"
#include "i386/isr.h"
#include "i386/idt.h"
#include "i386/v86/v86_monitor.h"

bool arch_init() {
    i386_gdt_initialize();
    i386_tss_initialize();
    i386_isr_initialize();
    i386_idt_initialize();
    return true;
}

bool arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc) {
    return v86_run_task(p_start, p_end, p_args, p_argc);
}

bool arch_is_virtualized() {
    outw(0x1ce, 0);
    return inw(0x1cf) >= 0xb0c0;
}

#endif