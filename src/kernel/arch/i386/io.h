#pragma once

#include <stdint.h>

#define __cdecl __attribute__((cdecl))

/**
 * @brief Disables interrupts and pauses forever. Only call if the OS requires to be panicked in such a way.
 */
void __cdecl i386_panic();

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port
 */
uint8_t __cdecl i386_inb(uint16_t p_port);

uint16_t __cdecl i386_inw(uint16_t p_port);

/**
 * @brief Sends a byte to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_value The data to send
 */
void __cdecl i386_outb(uint16_t p_port, uint8_t p_value);

/**
 * @brief Sends a word to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_value The data to send
 */
void __cdecl i386_outw(uint16_t p_port, uint16_t p_value);

/**
 * @brief Sets the value of a given MSR to the inputted results
 * @param p_msr The MSR to set
 * @param p_low The lower 32 bits of the value
 * @param p_high The higher 32 bits of the value
 */
void __cdecl i386_set_msr(uint32_t p_msr, uint32_t p_low, uint32_t p_high);

/**
 * @brief Gets the values held inside a given MSR
 * @param p_msr The MSR to read from
 * @param p_low The lower 32 bits to read in
 * @param p_high The higher 32 bits to read in
 */
void __cdecl i386_get_msr(uint32_t p_msr, uint32_t *p_low, uint32_t *p_high);
