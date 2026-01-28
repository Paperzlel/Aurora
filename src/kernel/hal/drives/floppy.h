#pragma once

#include <aurora/kdefs.h>

/**
 * @brief Initializes the floppy disk subsystem. Does not apply mountpoints to drives, as that is governed by the filesystem.
 */
void floppy_initialize();

/**
 * @brief Reads a number of bytes from the floppy disk into the output buffer. Since protected-mode floppies do not rely on per-sector reading, the 
 * amount of bytes available can be a number not equal to that for simplicity in smaller files. However, the offset cannot be specified, so reading smaller
 * files in parts will require correcting the offset used.
 * @param p_drive The drive number to use when reading
 * @param p_lba The Linear Block Address (LBA) to begin reading from
 * @param p_to The output buffer to write data into
 * @param p_size The number of bytes to read in
 * @return The pointer to the output buffer, which should be the same after writing, and `NULL` on failure.
 */
void *floppy_read(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size);

/**
 * @brief Writes a number of bytes from a buffer onto a floppy disk starting from a given sector. It only writes data to disk and does not update dates, nor
 * does it create directory info or write location data to the FAT (if using FAT). As with reading, it also starts from a beginning sector and as such is
 * stuck to 512-byte (or whatever the sector size is) intervals, and data that isn't re-written to this file will be overwritten.
 * @param p_drive The drive ID of which to write to.
 * @param p_lba The starting LBA in which to begin the read
 * @param p_from The buffer in which to read information from
 * @param p_size The number of bytes to write into the floppy
 * @return `true` if the command succeeded, and `false` if it failed.
 */
bool floppy_write(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size);

/**
 * @brief Gets the number of floppy drives detected on the system. Floppies booting via USB may not count towards this (has not been tested on an actual PC)
 * @return The number of drives found. Zero if not yet created.
 */
uint8_t floppy_get_drive_count();

/**
 * @brief Set the desired address and size of the data to transfer from the floppy disk to the system memory and vice versa. Data is stored in a 
 * 24-byte pattern of PHYSICAL memory - the DMA bus bypasses the CPU as we cannot translate addresses.
 * @param p_address The physical address to transfer data to.
 * @param p_size The size of the memory being used.
 */
void floppy_dma_setup_for_location(void *p_address, uint16_t p_size);

/**
 * @brief Sets the floppy DMA to begin recieving bytes from the disk to the memory. 
 */
void floppy_dma_read();

/**
 * @brief Sets the  desired address to write data from the system memory to the floppy disk. Data is stored in a 24-byte pattern of PHYSICAL memory -
 * the DMA bus bypasses the CPU as we cannot translate addresses.
 * @param p_address The physical address to transfer data from.
 * @param p_size The size of the memory to transfer.
 */
void floppy_dma_write();
