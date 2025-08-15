#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t drive_id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

bool disk_initialize(DISK *p_disk, uint8_t p_drive_no);

void disk_lba_to_chs(DISK *p_disk, uint16_t lba, uint16_t *p_out_cylinder, uint16_t *p_out_sector, uint16_t *p_out_head);

bool disk_read_sectors(DISK *p_disk, uint16_t lba, uint16_t p_sector_count, void *p_out_data);