#include <aurora/hal/hal.h>

#include "drives/floppy.h"

#include <sys/time.h>

/* Pre-define needed HAL functions without needing to add a header */

extern void pic_initialize();
extern void kbd_initialize();

extern void pit_initialize();
extern uint64_t pit_get_ticks();
extern uint32_t pit_get_frequency();

void hal_initialize(uint16_t p_driver_no)
{
    // Initialize the PIC first to get all interrupts going
    pic_initialize();
    // Then initialize PIT to get a logging timer
    pit_initialize();
    // Handle keyboard interrupts as well
    kbd_initialize();

    // Enable client interrupts again, to begin collecting timer info and allow us to use IRQ6 for the floppy disk
    __asm__ volatile ("sti");
    
    if (p_driver_no < 0x80)
    {
        // Initialize FDC
        floppy_initialize();
    }
    else
    {
        // Initialize hard disk controller
    }

}

uint64_t hal_get_ticks()
{
    return pit_get_ticks();
}


uint8_t hal_get_drive_count()
{
    // Add HDD to this as well
    return floppy_get_drive_count();
}


void *hal_read_bytes(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size)
{
    if (p_drive < 0x80)
    {
        return floppy_read(p_drive, p_lba, p_to, p_size);
    }
    
    return NULL; // HDD reading
}


bool hal_write_bytes(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size)
{
    if (p_drive < 0x80)
    {
        return floppy_write(p_drive, p_lba, p_from, p_size);
    }

    return false;   // HDD writing
}


bool timer_get_time(timer_t *p_timer)
{
    p_timer->ticks = pit_get_ticks();
    uint32_t frequency_ms = pit_get_frequency() / 1000;
    if (frequency_ms == 0)
    {
        return false;
    }

    // If 10 ticks pass, then 1 / 10000 * 10 = 1 / 1000 = 0.001s = 1ms has passed
    // Number of MS is equal to the tick count divided by the frequency divided by 1000
    // Number of us is equal to the tick count mod the frequency divided by 1000
    p_timer->time_ms = p_timer->ticks / frequency_ms;
    p_timer->time_us = p_timer->ticks % frequency_ms;

    return true;
}

void timer_sleep(uint64_t p_ms)
{
    uint64_t ticks_start = pit_get_ticks();
    uint32_t frequency_ms = pit_get_frequency() / 1000;
    if (!frequency_ms)
    {
        return;
    }

    while (true)
    {
        if (((pit_get_ticks() - ticks_start) / frequency_ms) >= p_ms)
        {
            break;
        }
    }
}
