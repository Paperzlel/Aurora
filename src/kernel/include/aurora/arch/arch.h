#ifndef _AURORA_ARCH_H
#define _AURORA_ARCH_H

#include "archdefs.h" // IWYU pragma: keep Needed for most implementations of architecture functions.

#include <aurora/kdefs.h>

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
 * @brief Runs a virtual 8086 task for the kernel. Tasks should always be written in 16-bit mode and avoid using 32-bit registers as it can
 * throw off the CPU (as it still executes them as 32-bit). Tasks should also run prior to entering userspace, for safety reasons.
 * @param p_start The start address of the program in memory
 * @param p_end The end address of the program in memory
 * @param p_args An array of words that are pushed onto the program stack in order of appearance. This means one should push their items
 * in reverse order onto the stack.
 * @param p_argc The number of words to push to the stack.
 * @return The return value for the application. `true` on success, `false` on failure. Applications should always ensure that the first item
 * on the stack (at 0x7b00) is the return code.
 */
bool arch_run_v86_task(void *p_start, void *p_end, uint16_t *p_args, int p_argc);

#endif // _AURORA_ARCH_H