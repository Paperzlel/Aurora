#ifndef _AURORA_HAL_H
#define _AURORA_HAL_H

#include <aurora/kdefs.h>

/**
 * @brief Initializes the Hardware Abstraction Layer, the part of the kernel that separates the hardware functions from the software implementation.
 * Differs from the CPU architecture in that the hardware available to one PC will be different to that of another and as such must be abstracted
 * differently.
 * @param p_drive_no The drive ID used to locate the OS on-disk.
 */
void hal_initialize(uint16_t p_driver_no);

uint64_t hal_get_ticks();

#endif // _AURORA_HAL_H