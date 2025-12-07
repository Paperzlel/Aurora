#include "disk.h"
#include "x86.h"

bool disk_initialize(DISK *p_disk, uint8_t p_drive_no) {
    uint8_t drive_type = 0;
    uint16_t cylinders, sectors, heads;

    if (!x86_Drive_GetDriveParameters(p_drive_no, &drive_type, &cylinders, &sectors, &heads)) {
        return false;
    }

    p_disk->drive_id = p_drive_no;
    p_disk->cylinders = cylinders;
    p_disk->sectors = sectors;
    p_disk->heads = heads;

    return true;
}

void disk_lba_to_chs(DISK *p_disk, uint16_t lba, uint16_t *p_out_cylinder, uint16_t *p_out_sector, uint16_t *p_out_head) {
    // LBA % sectors_per_track + 1 = sector
    *p_out_sector = (lba % p_disk->sectors) + 1;

    // (LBA / sectors_per_track) / Heads = cylinder
    *p_out_cylinder = lba / (p_disk->sectors * p_disk->heads);

    // (LBA / sectors_per_track) % Heads = head
    *p_out_head = (lba % (p_disk->sectors * p_disk->heads)) / p_disk->sectors;
}

bool disk_read_sectors(DISK *p_disk, uint16_t lba, uint16_t p_sector_count, void *p_out_data) {
    uint16_t cylinder, sector, head;

    disk_lba_to_chs(p_disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++) {
        if (x86_Drive_ReadDisk(p_sector_count, p_disk->drive_id, cylinder, sector, head, p_out_data)) {
            return true;
        }

        x86_Drive_ResetDisk(p_disk->drive_id);
    }

    return false;
}