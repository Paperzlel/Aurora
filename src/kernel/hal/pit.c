#include <aurora/arch/interrupts.h>
#include <asm/io.h>

#include <stdio.h>

#define PIT_DATA_CHANNEL_0      0x40
#define PIT_DATA_CHANNEL_1      0x41
#define PIT_DATA_CHANNEL_2      0x42
#define PIT_COMMAND_REGISTER    0x43

#define PIT_BASE_FREQUENCY  1193182
// Slowest possible timer frequency (18Hz)
#define PIT_MIN_FREQUENCY PIT_BASE_FREQUENCY / 19
// Highest possible timer frequency (1193182Hz)
#define PIT_MAX_FREQUENCY PIT_BASE_FREQUENCY / PIT_BASE_FREQUENCY

static uint64_t tick_count;
static uint32_t frequency;
static bool initialized = false;

bool pit_irq_handler(struct Registers *p_regs)
{
    if (initialized)
    {
        tick_count++;
    }

    send_end_of_interrupt(p_regs->interrupt);
    return true;
}

void pit_initialize()
{
    register_interrupt_handler(INT_IRQ_0, pit_irq_handler);
    
    // Set timer to tick every 100us (~119us)
    frequency = 10000;
    uint16_t divisor = (uint16_t)((PIT_BASE_FREQUENCY / frequency) * 2);
    uint8_t low = (uint8_t)(divisor & 0xff);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xff);

    // Set channel 0, access low+high byte, mode 3
    outb(PIT_COMMAND_REGISTER, (3 << 4) | (3 << 1));

    outb(PIT_DATA_CHANNEL_0, low);
    outb(PIT_DATA_CHANNEL_0, high);

    initialized = true;
    // Unmask timer IRQ, ready to handle interrupts!
    unmask_irq(INT_IRQ_0);
}

uint64_t pit_get_ticks()
{
    return tick_count;
}

uint32_t pit_get_frequency()
{
    return frequency;
}
