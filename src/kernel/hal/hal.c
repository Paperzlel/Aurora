#include <kernel/hal/hal.h>
#include "pit.h"
#include "pic.h"
#include "kbd.h"

#include "drives/floppy.h"

#include <sys/time.h>

void hal_initialize(uint16_t p_driver_no) {
    // Initialize the PIC first to get all interrupts going
    pic_initialize();
    // Then initialize PIT to get a logging timer
    pit_initialize();
    // Handle keyboard interrupts as well
    kbd_initialize();

    // Enable client interrupts again, to begin collecting timer info and allow us to use IRQ6 for the floppy disk
    __asm__ volatile ("sti");
    
    if (p_driver_no < 0x80) {
        // Initialize FDC
        floppy_initialize();
    } else {
        // Initialize hard disk controller
    }

}

uint64_t hal_get_ticks() {
    return pit_get_ticks();
}

bool timer_get_time(timer_t *p_timer) {
    p_timer->ticks = pit_get_ticks();
    uint32_t frequency_ms = pit_get_frequency() / 1000;
    if (frequency_ms == 0) {
        return false;
    }

    // If 10 ticks pass, then 1 / 10000 * 10 = 1 / 1000 = 0.001s = 1ms has passed
    // Number of MS is equal to the tick count divided by the frequency divided by 1000
    // Number of us is equal to the tick count mod the frequency divided by 1000
    p_timer->time_ms = p_timer->ticks / frequency_ms;
    p_timer->time_us = p_timer->ticks % frequency_ms;

    return true;
}

void timer_sleep(uint64_t p_ms) {
    uint64_t ticks_start = pit_get_ticks();
    uint32_t frequency_ms = pit_get_frequency() / 1000;
    if (!frequency_ms) {
        return;
    }

    while (true) {
        if (((pit_get_ticks() - ticks_start) / frequency_ms) >= p_ms) {
            break;
        }
    }
}
