#include "fat.h"
#include "memdefs.h"

#include "stdio.h"
#include "string.h"
#include "memory.h"

#include <stddef.h>

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 8
#define ROOT_DIRECTORY_HANDLE -1

typedef struct {
    uint8_t boot_jump[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_table_count;
    uint16_t root_dir_entry_count;
    uint16_t total_sector_count;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors;

    uint8_t drive_no;
    uint8_t reserved;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];
} __attribute__((packed)) FAT_BootSector;


typedef struct {
    uint8_t buffer[SECTOR_SIZE];
    FAT_File public_fa;
    bool opened;

    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;
} FAT_FileData;

typedef struct {
    union {
        FAT_BootSector boot_sector;
        uint8_t data[SECTOR_SIZE];
    } bs;

    FAT_FileData root_directory;

    FAT_FileData open_handles[MAX_FILE_HANDLES];
} FAT_Data;

static FAT_Data *a_filesystem;
static uint8_t *a_fat_data = NULL;
static uint32_t a_data_section_lba;


bool fat_read_boot_sector(DISK *p_disk) {
    return disk_read_sectors(p_disk, 0, 1, a_filesystem->bs.data);
}

bool fat_read_fat_table(DISK *p_disk) {
    return disk_read_sectors(p_disk, a_filesystem->bs.boot_sector.reserved_sector_count, a_filesystem->bs.boot_sector.sectors_per_fat, a_fat_data);
}

bool fat_initialize(DISK *p_disk) {
    a_filesystem = (FAT_Data *)MEMORY_FAT_FS;

    if (!fat_read_boot_sector(p_disk)) {
        printf("FAT: Failed to read boot sector into memory.\n");
        return false;
    }

    a_fat_data = (uint8_t *)a_filesystem + sizeof(FAT_Data);
    uint32_t fat_size = a_filesystem->bs.boot_sector.bytes_per_sector * a_filesystem->bs.boot_sector.sectors_per_fat;
    if (sizeof(FAT_Data) + fat_size >= MEMORY_FAT_SIZE) {
        printf("FAT: Not enough memory to read all FAT data in. Needs %lu, has %u.\n", sizeof(FAT_Data) + fat_size, MEMORY_FAT_SIZE);
        return false;
    } else {
        printf("FAT: Allocated %lu bytes for the FAT data entries.\n", sizeof(FAT_Data) + fat_size);
    }

    if (!fat_read_fat_table(p_disk)) {
        printf("FAT: Failed to read FAT.\n");
        return false;
    }

    uint32_t root_dir_lba = a_filesystem->bs.boot_sector.fat_table_count * a_filesystem->bs.boot_sector.sectors_per_fat;
    root_dir_lba += a_filesystem->bs.boot_sector.reserved_sector_count;

    a_filesystem->root_directory.public_fa.handle = ROOT_DIRECTORY_HANDLE;
    a_filesystem->root_directory.public_fa.is_directory = true;
    a_filesystem->root_directory.public_fa.position = 0;
    a_filesystem->root_directory.public_fa.size = sizeof(FAT_DirectoryEntry) * a_filesystem->bs.boot_sector.root_dir_entry_count;
    a_filesystem->root_directory.first_cluster = root_dir_lba;
    a_filesystem->root_directory.current_cluster = root_dir_lba;
    a_filesystem->root_directory.current_sector_in_cluster = 0;

    if (!disk_read_sectors(p_disk, root_dir_lba, 1, a_filesystem->root_directory.buffer)) {
        printf("FAT: Failed to read root directory.\n");
        return false;
    }

    uint32_t root_dir_sectors = (a_filesystem->bs.boot_sector.root_dir_entry_count << 5) + (a_filesystem->bs.boot_sector.bytes_per_sector - 1);
    root_dir_sectors /= a_filesystem->bs.boot_sector.bytes_per_sector;

    a_data_section_lba = root_dir_lba + root_dir_sectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        a_filesystem->open_handles[i].opened = false;
    }

    return true;
}

FAT_File *fat_open(DISK *p_disk, const char *p_path) {

    // 255-byte filename + NULL terminator. Here we copy in the file 
    char name[MAX_PATH_SIZE];

    // e.g. find "/dev/test.txt"

    // Remove '/' from start of file
    // Extract "dev" from path (strchr(path, '/'), then sub result from path to get dev on its own)
    // Look in current directory for dev (toupper each letter, move to 12-byte NULL-terminated buffer, memcmp with filename)
    // Find it, OR in attribs, load "folder", return new directory
    // Open new directory entry, move along
    
    // Extract "test.txt" from path (same as above, should return NULL as the '/' was removed in the last loop)
    // Look in current directory for file (toupper name and ext, mov to buffer, memcmp with each directory filename)
    // Find, OR in attribs, load directory, return
    // path should be NULL now, return

    if (p_path[0] == '/') {
        p_path++;
    }

    FAT_File *current = &a_filesystem->root_directory.public_fa;

    while (*p_path) {
        bool is_last = false;

        const char *delim = strchr(p_path, '/');
        if (delim != NULL) {
            memcpy(name, p_path, delim - p_path);
            name[delim - p_path + 1] = 0;
            p_path = delim + 1;
        } else {
            uint32_t len = strlen(p_path);
            memcpy(name, p_path, len);
            p_path += len;
            is_last = true;
        }

        // Load a file handler somehow
        FAT_DirectoryEntry entry;
        printf("name is now: %s\n", name);
    }

    return current;
}