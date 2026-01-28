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


/**
 * @brief Obtains the number of usable drives connected to the PC, not including USB devices. 
 * @return The number of usable drives connected to the PC
 */
uint8_t hal_get_drive_count();

/**
 * @brief Reads N bytes from a drive into a buffer. Implementation depends on the drive in question, which are handled differently according to their needs.
 * @param p_drive The drive to read from
 * @param p_lba The LBA to begin reading from
 * @param p_to The output buffer to read information into
 * @param p_size The number of bytes to read
 * @return The pointer passed in by the user now filled with information, or `NULL` if something failed.
 */
void *hal_read_bytes(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size);

/**
 * @brief Writes N bytes from an input buffer onto a drive starting at a given LBA. Implementation depends on the drive in question, however most use-cases of
 * this should follow a similar pattern.
 * @param p_drive The drive to read from
 * @param p_lba The LBA to begin reading from. LBAs refer to whole sectors and read/writes cannot begin from an offset into a sector.
 * @param p_from The buffer to write data from
 * @param p_size The number of bytes contained in the buffer
 * @return `true` on success, `false` if an error occured.
 */
bool hal_write_bytes(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size);

#endif // _AURORA_HAL_H