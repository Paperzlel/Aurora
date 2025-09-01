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
void arch_run_v86_task(void *p_start, void *p_end, uint8_t *p_args, int p_argc);