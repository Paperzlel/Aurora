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

void arch_io_outw(uint16_t p_port, uint16_t p_value);