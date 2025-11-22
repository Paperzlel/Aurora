#include "kbd.h"

#include <arch/arch.h>

bool kbd_handler(Registers *p_regs) {
    arch_send_eoi(p_regs->interrupt);
    return true;
}

void kbd_initialize() {
    arch_register_isr_handler(0x21, kbd_handler);
}