#ifndef _AURORA_ARCH_H
#define _AURORA_ARCH_H

#include <aurora/kdefs.h>
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

#endif // _AURORA_ARCH_H