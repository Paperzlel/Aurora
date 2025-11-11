#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Checks CPU architecture and loads the resepective backend for it.
 */
bool arch_init();

/**
 * @brief Runs a virtual 8086 task for the kernel.
 * @param p_start The start address of the program in memory
 * @param p_end The end address of the program in memory
 */
bool arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc);

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port
 */
uint8_t arch_io_inb(uint16_t p_port);

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port.
 */
uint16_t arch_io_inw(uint16_t p_port);

/**
 * @brief Sends a byte to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_value The data to send
 */
void arch_io_outb(uint16_t p_port, uint8_t p_value);

/**
 * @brief Sends a word to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_value The data to send
 */
void arch_io_outw(uint16_t p_port, uint16_t p_value);

/**
 * @brief Sets the value of a given MSR to the inputted results
 * @param p_msr The MSR to set
 * @param p_low The lower 32 bits of the value
 * @param p_high The higher 32 bits of the value
 */
void arch_set_msr(uint32_t p_msr, uint32_t p_lower, uint32_t p_higher);

/**
 * @brief Gets the values held inside a given MSR
 * @param p_msr The MSR to read from
 * @param p_low The lower 32 bits to read in
 * @param p_high The higher 32 bits to read in
 */
void arch_get_msr(uint32_t p_msr, uint32_t *p_lower, uint32_t *p_higher);

/**
 * @brief Checks to see if the user is running on bochs or another emulator that supports bochs VBE drivers.
 * @returns True if bochs VBE is supported, false if not.
 */
bool arch_is_virtualized();