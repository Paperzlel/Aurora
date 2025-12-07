#include <aurora/arch/interrupts.h>

bool kbd_handler(struct Registers *p_regs)
{
    send_end_of_interrupt(p_regs->interrupt);
    return true;
}

void kbd_initialize()
{
    register_interrupt_handler(0x21, kbd_handler);
}
