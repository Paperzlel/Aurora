#include "hal.h"
#include "pit.h"
#include "pic.h"
#include "kbd.h"

void hal_initialize() {
    // Initialize the PIC first to get all interrupts going
    pic_initialize();
    // Then initialize PIT to get a logging timer
    pit_initialize();
    // Handle keyboard interrupts as well
    kbd_initialize();
    // Enable client interrupts again. This should happen as late as possible.
    __asm__ volatile ("sti");
}


uint64_t hal_get_ticks() {
    return pit_get_ticks();
}

void hal_get_time_relative(timer_t *p_timer) {
    p_timer->ticks = pit_get_ticks();
    uint32_t frequency_ms = pit_get_frequency() / 1000;

    // If 10 ticks pass, then 1 / 10000 * 10 = 1 / 1000 = 0.001s = 1ms has passed
    // Number of MS is equal to the tick count divided by the frequency divided by 1000
    // Number of us is equal to the tick count mod the frequency divided by 1000
    p_timer->time_ms = p_timer->ticks / frequency_ms;
    p_timer->time_us = p_timer->ticks % frequency_ms;
}