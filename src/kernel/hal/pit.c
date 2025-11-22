#include "pit.h"
#include "pic.h"

#include <arch/arch.h>
#include <arch/io.h>

#define PIT_DATA_CHANNEL_0      0x40
#define PIT_DATA_CHANNEL_1      0x41
#define PIT_DATA_CHANNEL_2      0x42
#define PIT_COMMAND_REGISTER    0x43

#define PIT_BASE_FREQUENCY  1193182
// Slowest possible timer frequency (18Hz)
#define PIT_MIN_FREQUENCY PIT_BASE_FREQUENCY / 65536 
// Highest possible timer frequency (596591Hz or ~0.6MHz)
#define PIT_MAX_FREQUENCY PIT_BASE_FREQUENCY / 2
// We can't use the base 1.1MHz because the timer mode we use (mode 3) reduces the count by 2 and thus a reload value of 1 would not work.
// Using the highest frequency makes QEMU unhappy. Avoid using that for now.

static uint64_t tick_count;
static uint32_t frequency;
static bool initialized = false;

bool pit_irq_handler(Registers *p_regs) {
    if (initialized) {
        tick_count++;
    }

    arch_send_eoi(p_regs->interrupt);
    return true;
}

void pit_initialize() {
    arch_register_isr_handler(0x20, pit_irq_handler);

    frequency = PIT_MIN_FREQUENCY;
    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / (frequency));
    uint8_t low = (uint8_t)(divisor & 0xff);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xff);

    // Set channel 0, access low+high byte, mode 3
    outb(PIT_COMMAND_REGISTER, (3 << 4) | (3 << 1));

    outb(PIT_DATA_CHANNEL_0, low);
    outb(PIT_DATA_CHANNEL_0, high);

    initialized = true;
    // Unmask timer IRQ, ready to handle interrupts!
    pic_unmask_irq(IRQ_0);
}

uint64_t pit_get_ticks() {
    return tick_count;
}