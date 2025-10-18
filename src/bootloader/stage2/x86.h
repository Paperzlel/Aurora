#pragma once
#include <stdint.h>
#include <stdbool.h>

#define __cdecl __attribute__((cdecl))

/**
 * @brief Sends a byte to the given I/O port.
 * @param p_port The port ID to send the data to
 * @param p_data The data to send
 */
void __cdecl x86_outb(uint16_t p_port, uint8_t p_data);

/**
 * @brief Returns data from a given port, if any.
 * @param p_port The port to request data from
 * @returns The data from the port
 */
uint8_t __cdecl x86_inb(uint16_t p_port);

/**
 * @brief Obtains information about a given drive, mainly its type and CHS count.
 * @param p_drive_no The drive to query information about
 * @param out_drive_type The drive "type" (size of the floppy disk)
 * @param out_cylinder_count Number of cylinders in the drive
 * @param out_sector_count Number of sectors in the drive
 * @param out_head_count Number of heads in the drive
 * @returns True if the operation was successful, and false if not.
 */
bool __cdecl x86_Drive_GetDriveParameters(uint8_t p_drive_no, 
            uint8_t *out_drive_type, 
            uint16_t *out_cylinder_count, 
            uint16_t *out_sector_count,
            uint16_t *out_head_count);

/**
 * @brief Reads n sectors from the disk into the given pointer. CHS must be less than the limits described above.
 * @param p_sector_count Number of sectors to read data from
 * @param p_drive_no The drive to read from
 * @param p_cylinder The starting cylinder to read data from
 * @param p_sector The starting sector to read data from
 * @param p_head The starting head to read data from
 * @returns True if successful, and false if not.
 */
bool __cdecl x86_Drive_ReadDisk(uint8_t p_sector_count, 
            uint8_t p_drive_no, 
            uint8_t p_cylinder, 
            uint8_t p_sector, 
            uint8_t p_head, 
            void *out_data);

/**
 * @brief Resets the disk controller and attempts to read the data again. Real-world floppy disks tend to fail, and as such we call this function if the
 * data could not be read the first time if the disk was running incorrectly.
 * @param p_drive_no The drive number to reset
 * @returns True if successful, and false if not.
 */
bool __cdecl x86_Drive_ResetDisk(uint8_t p_drive_no);

/**
 * @brief Obtains data describing a memory region. This memory is not in order of greater addresses and as a result may have some errors.
 * @param p_region The current region to find memory information for
 * @param p_out_region The next region to obtain data about, 0 when the function has reached the end of the chain.
 * @param p_out_data Pointer to a struct to contain the data within
 * @returns The size of the data written into the pointer. Either 20 or 24 bytes, depending on the BIOS.
 */
uint8_t __cdecl x86_Memory_GetMemoryRegion(uint16_t p_region,
            uint16_t *p_out_region,
            void *p_out_data);

/**
 * @brief Obtains information about the current VESA VBE BIOS for this PC's GPU.
 * @param p_struct The struct to fill out the data into. NOTE: Must be 512 bytes large.
 * @returns True if successful, false if the function is unsupported or if there was an error in obtaining the data.
 */
bool __cdecl x86_VBE_GetVESAInfo(void *p_struct);

/**
 * @brief Obtains information about the given graphics mode and fills out a pointer with it. The graphics mode must be one returned by x86_VBE_GetVESAInfo()
 * or the function is unlikely to work.
 * @param p_video_mode The video mode to obtain information about
 * @param p_out_data Pointer to a struct to fill the data of
 * @returns True if successful, false if the function is unsupported or if there was an error in obtaining the data.
 */
bool __cdecl x86_VBE_GetVESAVideoModeInfo(uint16_t p_video_mode,
            void *p_out_data);

bool __cdecl x86_EDID_GetVideoBlock(void *p_out_data);