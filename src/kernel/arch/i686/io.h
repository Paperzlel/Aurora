#pragma once

#include <stdint.h>

#define __cdecl __attribute__((cdecl))

/**
 * @brief Disables interrupts and pauses forever. Only call if the OS requires to be panicked in such a way.
 */
void __cdecl i686_panic();

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port
 */
uint8_t __cdecl i686_inb(uint16_t p_port);

uint16_t __cdecl i686_inw(uint16_t p_port);

/**
 * @brief Sends a byte to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_value The data to send
 */
void __cdecl i686_outb(uint16_t p_port, uint8_t p_value);

void __cdecl i686_outw(uint16_t p_port, uint16_t p_value);