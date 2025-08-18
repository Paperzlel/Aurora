#pragma once

#include <stdint.h>
#include <stdbool.h>

// Represents a floppy disk, which is used to load data using the interrupt INT 0x13 for our kernel
typedef struct {
    uint8_t drive_id;           // The drive number (0x0 --> 0x7F for floppy disks, 0x80+ for hard drives/SSDs)
    uint16_t cylinders;         // No. of cylinders
    uint16_t sectors;           // No. of sectors
    uint16_t heads;             // No. of heads
} DISK;

/**
 * @brief Initializes a disk structure by querying for its parameters and filling in the subsequent struct. Any disk operations require this code.
 * @param p_disk The disk structure to initialize
 * @param p_drive_no The drive number to use for the disk
 * @returns True if the drive could be queried, and false if there was a read error, or if the drive ID does not exist.
 */
bool disk_initialize(DISK *p_disk, uint8_t p_drive_no);

/**
 * @brief Converts a given Logical Block Address (conventional scheme for most drives) to a Cylinder-Sector-Head format (used by floppy disks)
 * @param p_disk The disk to read from
 * @param lba The Logical Block Address that we wish to convert
 * @param p_out_cylinder Pointer to the cylinder no.
 * @param p_out_sector Pointer to the sector no.
 * @param p_out_head Pointer to the head no.
 */
void disk_lba_to_chs(DISK *p_disk, uint16_t lba, uint16_t *p_out_cylinder, uint16_t *p_out_sector, uint16_t *p_out_head);

/**
 * @brief Reads n sectors from the disk into the output buffer, starting with sector lba - 1. 
 * Each sector is 512 bytes large and may contain less data than this amount. 
 * @param p_disk The disk to read from
 * @param lba The starting Logical Block Address to read from
 * @param p_sector_count The number of sectors to read from
 * @param p_out_data Pointer to the place where bytes should be written to.
 * @returns True if the operation succeeded, and false if the drive could not be read from, either due to disk issues or due to an invalid LBA/Sector count.
 */
bool disk_read_sectors(DISK *p_disk, uint16_t lba, uint16_t p_sector_count, void *p_out_data);