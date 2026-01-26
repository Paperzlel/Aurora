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

uint8_t hal_get_drive_count();

void *hal_read_bytes(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size);
bool hal_write_bytes(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size);

#endif // _AURORA_HAL_H