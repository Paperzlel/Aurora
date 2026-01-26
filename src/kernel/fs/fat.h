#pragma once

#include <aurora/kdefs.h>

extern bool fat_initialize(uint8_t p_drive_no, void *p_bootsector);

/**
 * @brief Opens a handle to the given FAT file. This handle is managed internally by the FAT driver.
 * @param p_file The path to the file that we want to load.
 * @param p_drive_id The ID of the drive to read.
 * @return The handle to the FAT file.
 */
extern void *fat_open(const char *p_file, uint8_t p_drive_id);


/**
 * @brief Obtains the size of the given file handle. 
 * @param p_handle The corresponding file handle
 * @return The size of the given file handle.
 */
extern int fat_get_size(void *p_handle);

/**
 * @brief Obtains the position of the given file handle relative to the beginning of the file.
 * @param p_handle The corresponding file handle
 * @return The relative position of the file handle.
 */
extern int fat_get_position(void *p_handle);