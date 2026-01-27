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
 * @brief Reads a given number of bytes from memory into a given buffer. If the region of memory has not yet been loaded, it attempts
 * to load it from disk into a file buffer.
 * @param p_file The corresponding file to load information about.
 * @param p_bytes The number of bytes requested to be loaded.
 * @param out_buffer A reference to the output buffer in which to read memory into. If it points to a `NULL` pointer, it assumes the
 * user wants to use the same data as the file buffer, which will halve memory usage but at the cost of potentially invalid 
 * memory access.
 * @return The number of bytes read, which should always be equal to `p_bytes`.
 */
extern uint32_t fat_read_bytes(void *p_handle, uint32_t p_bytes, void **out_buffer);

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