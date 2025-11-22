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