#pragma once

#include <kdefs.h>
#include "archdefs.h"

/**
 * @brief Checks CPU architecture and loads the resepective backend for it.
 */
bool arch_init();

/**
 * @brief Checks to see if the user is running on bochs or another emulator that supports bochs VBE drivers.
 * @returns True if bochs VBE is supported, false if not.
 */
bool arch_is_virtualized();

/**
 * @brief Runs a virtual 8086 task for the kernel.
 * @param p_start The start address of the program in memory
 * @param p_end The end address of the program in memory
 */
bool arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc);

/**
 * @brief Registers an ISR handler to a given ISR. Since there are many different services to look for, this method needs to be exposed.
 * @param p_isr The ISR to bind to
 * @param p_handle Function pointer to where we will handle the data externally
 * @return `true` if successful, and `false` if there already was an ISR there in the first place.
 */
bool arch_register_isr_handler(uint8_t p_isr, InterruptHandler p_handle);

void arch_send_eoi(uint8_t p_irq);