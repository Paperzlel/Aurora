#include "isr.h"
#include "idt.h"
#include "io.h"

#include <stdio.h>

// List of error codes that we can handle
const char *a_exception_errors[22] = {
    "Divide by zero error",
    "Debug exception",
    "NMI Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Math coprocessor not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not found",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "",
    "Floating-point error",
    "Alignment fault",
    "Machine check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Control process exception"
};

/**
 * @brief Dummy definition to the function in "isr_list.c". Needed so we don't include it and get redefinition errors.
 */
void i686_idt_register_isrs();

/**
 * @brief Default interrupt handler. Will likely need a system to register more handlers in the future,.
 * @param p_regs Registers pushed to the stack by our ASM interrupt handler.
 */
void __attribute__((cdecl)) i686_interrupt_handler(Registers *p_regs) {
    if (p_regs->interrupt < 22) {
        printf("Unhandled exception %d: %s\n", p_regs->interrupt, a_exception_errors[p_regs->interrupt]);
    } else {
        printf("Unhandled exception %d\n", p_regs->interrupt);
    }

    printf("  eax=%x  ebx=%x  ecx=%x  edx=%x\n", p_regs->eax, p_regs->ebx, p_regs->ecx, p_regs->edx);
    printf("  esi=%x  edi=%x\n", p_regs->esi, p_regs->edi);
    printf("  esp=%x  ebp=%x  eip=%x\n", p_regs->esp, p_regs->ebp, p_regs->eip);
    printf("  eflags=%x  cs=%x ds=%x ss=%x\n", p_regs->eflags, p_regs->cs, p_regs->ds, p_regs->ss);
    printf("  interrupt=%x error_code=%x\n", p_regs->interrupt, p_regs->error);

    printf("KERNEL PANIC\n");
    
    i686_panic();
}

void i686_isr_initialize() {
    i686_idt_register_isrs();

    for (int i = 0; i < 256; i++) {
        i686_idt_enable_isr(i);
    }
}