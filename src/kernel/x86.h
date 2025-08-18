#pragma once
#include <stdint.h>
#include <stdbool.h>

#define __cdecl __attribute__((cdecl))

/**
 * @brief Sends a byte to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_data The data to send
 */
void __cdecl x86_outb(uint16_t p_port, uint8_t p_data);

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port
 */
uint8_t __cdecl x86_inb(uint16_t p_port);