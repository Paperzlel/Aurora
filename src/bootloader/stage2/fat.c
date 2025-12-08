#include "fat.h"
#include "memdefs.h"

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include "minmax.h"

#include <stddef.h>

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 8
#define ROOT_DIRECTORY_HANDLE -1

struct __attribute__((packed)) FAT_BootSector
{
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
};


struct FAT_FileData
{
    uint8_t buffer[SECTOR_SIZE];
    struct FAT_File public_fa;
    bool opened;

    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;
};

struct FAT_Data
{
    union
    {
        struct FAT_BootSector boot_sector;
        uint8_t data[SECTOR_SIZE];
    } bs;

    struct FAT_FileData root_directory;

    struct FAT_FileData open_handles[MAX_FILE_HANDLES];
};

static struct FAT_Data *a_filesystem;
static uint8_t *a_fat_data = NULL;
static uint32_t a_data_section_lba;


bool fat_read_boot_sector(struct DISK *p_disk)
{
    return disk_read_sectors(p_disk, 0, 1, a_filesystem->bs.data);
}

bool fat_read_fat_table(struct DISK *p_disk)
{
    return disk_read_sectors(p_disk, a_filesystem->bs.boot_sector.reserved_sector_count, a_filesystem->bs.boot_sector.sectors_per_fat, a_fat_data);
}

uint32_t fat_cluster_to_lba(uint32_t p_in)
{
    return ((p_in - 2) * a_filesystem->bs.boot_sector.sectors_per_cluster + a_data_section_lba);
}

uint32_t fat_next_cluster(uint32_t p_input_sector)
{
    uint32_t offset = p_input_sector * 3 / 2;
    uint16_t ret = (*(uint16_t *)(a_fat_data + offset));

    return (p_input_sector & 1) ? ret >> 4 : ret & 0xfff;
}

bool fat_find_entry(struct DISK *p_disk, struct FAT_File *p_current, const char *p_name, struct FAT_DirectoryEntry *p_directory)
{
    struct FAT_DirectoryEntry entry;
    char fat_filename[12];
    memset(fat_filename, ' ', 12);
    fat_filename[11] = 0;

    // Obtain the file extension (NULL if it's a directory)
    const char *ext = strchr(p_name, '.');
    if (ext == NULL)
    {
        ext = p_name + 11;
    }

    for (int i = 0; i < 8 && p_name[i] && p_name + i < ext; i++)
    {
        fat_filename[i] = toupper(p_name[i]);
    }

    if (ext != p_name + 11)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
        {
            fat_filename[i + 8] = toupper(ext[i + 1]);
        }
    }

    while (fat_read_dir_entry(p_disk, p_current, &entry))
    {
        if (memcmp(fat_filename, entry.file_name, 11) == 0)
        {
            *p_directory = entry;
            return true;
        }
    }

    return false;
}

struct FAT_File *fat_open_entry(struct DISK *p_disk, struct FAT_DirectoryEntry *p_directory)
{
    int handle = ROOT_DIRECTORY_HANDLE;

    for (int i = 0; i < MAX_FILE_HANDLES; i++)
    {
        if (!a_filesystem->open_handles[i].opened)
        {
            handle = i;
            break;
        }
    }

    if (handle == ROOT_DIRECTORY_HANDLE)
    {
        printf("FAT: No more file handles available.\n");
        return NULL;
    }

    struct FAT_FileData *data = &a_filesystem->open_handles[handle];
    data->public_fa.handle = handle;
    data->public_fa.is_directory = (p_directory->attribs & FAT_DIRECTORY) != 0;
    data->public_fa.position = 0;
    data->public_fa.size = p_directory->size;
    data->first_cluster = p_directory->first_cluster_no_low + ((uint32_t)p_directory->first_cluster_no_high << 16);
    data->current_cluster = data->first_cluster;
    data->current_sector_in_cluster = 0;

    if (!disk_read_sectors(p_disk, fat_cluster_to_lba(data->current_cluster), 1, data->buffer))
    {
        printf("FAT: Opening entry failed - read error on cluster %u (LBA %u)\n", data->current_cluster, fat_cluster_to_lba(data->current_cluster));
        for (int i = 0; i < 11; i++)
        {
            printf("%c", p_directory->file_name[i]);
        }
        printf("\n");
        return NULL;
    }

    data->opened = true;
    return &data->public_fa;
}

bool fat_initialize(struct DISK *p_disk)
{
    a_filesystem = (struct FAT_Data *)MEMORY_FAT_FS;

    if (!fat_read_boot_sector(p_disk))
    {
        printf("FAT: Failed to read boot sector into memory.\n");
        return false;
    }

    a_fat_data = (uint8_t *)a_filesystem + sizeof(struct FAT_Data);
    uint32_t fat_size = a_filesystem->bs.boot_sector.bytes_per_sector * a_filesystem->bs.boot_sector.sectors_per_fat;
    if (sizeof(struct FAT_Data) + fat_size >= MEMORY_FAT_SIZE)
    {
        printf("FAT: Not enough memory to read all FAT data in. Needs %lu, has %u.\n", sizeof(struct FAT_Data) + fat_size, MEMORY_FAT_SIZE);
        return false;
    }
    else
    {
        printf("FAT: Allocated %lu bytes for the FAT data entries.\n", sizeof(struct FAT_Data) + fat_size);
    }

    if (!fat_read_fat_table(p_disk))
    {
        printf("FAT: Failed to read FAT.\n");
        return false;
    }

    uint32_t root_dir_lba = a_filesystem->bs.boot_sector.fat_table_count * a_filesystem->bs.boot_sector.sectors_per_fat;
    root_dir_lba += a_filesystem->bs.boot_sector.reserved_sector_count;

    a_filesystem->root_directory.public_fa.handle = ROOT_DIRECTORY_HANDLE;
    a_filesystem->root_directory.public_fa.is_directory = true;
    a_filesystem->root_directory.public_fa.position = 0;
    a_filesystem->root_directory.public_fa.size = sizeof(struct FAT_DirectoryEntry) * a_filesystem->bs.boot_sector.root_dir_entry_count;
    a_filesystem->root_directory.first_cluster = root_dir_lba;
    a_filesystem->root_directory.current_cluster = root_dir_lba;
    a_filesystem->root_directory.current_sector_in_cluster = 0;
    printf("FAT: Setup root directory with size %d and first cluster no. %d.\n", a_filesystem->root_directory.public_fa.size, root_dir_lba);

    if (!disk_read_sectors(p_disk, root_dir_lba, 1, a_filesystem->root_directory.buffer))
    {
        printf("FAT: Failed to read root directory.\n");
        return false;
    }

    uint32_t root_dir_sectors = (a_filesystem->bs.boot_sector.root_dir_entry_count << 5) + (a_filesystem->bs.boot_sector.bytes_per_sector - 1);
    root_dir_sectors /= a_filesystem->bs.boot_sector.bytes_per_sector;

    a_data_section_lba = root_dir_lba + root_dir_sectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++)
    {
        a_filesystem->open_handles[i].opened = false;
    }

    return true;
}

struct FAT_File *fat_open(struct DISK *p_disk, const char *p_path)
{
    // 255-byte filename + NULL terminator. Here we copy in the file 
    char name[MAX_PATH_SIZE];

    if (p_path[0] == '/')
    {
        p_path++;
    }

    struct FAT_File *current = &a_filesystem->root_directory.public_fa;

    while (*p_path)
    {
        bool is_last = false;
        const char *delim = strchr(p_path, '/');

        if (delim != NULL)
        {
            memcpy(name, p_path, delim - p_path);
            name[delim - p_path + 1] = 0;
            p_path = delim + 1;
        }
        else
        {
            uint32_t len = strlen(p_path);
            memcpy(name, p_path, len);
            name[len] = 0;
            p_path += len;
            is_last = true;
        }

        // Load a file handler
        struct FAT_DirectoryEntry entry;
        if (!fat_find_entry(p_disk, current, name, &entry))
        {
            fat_close(current);

            printf("FAT: Could not find/read directory %s.\n", name);
            return NULL;
        }
        else
        {
            // printf("Found entry with name %s.\n", name);

            fat_close(current);

            if (!is_last && (entry.attribs & FAT_DIRECTORY) == 0)
            {
                printf("FAT: Entry %s is not a directory.\n", name);
                return NULL;
            }

            current = fat_open_entry(p_disk, &entry);
        }
    }

    return current;
}

uint32_t fat_read(struct DISK *p_disk, struct FAT_File *p_file, uint32_t p_bytes, void *p_out_data)
{
    // Get filedata handle
    struct FAT_FileData *data = (p_file->handle == ROOT_DIRECTORY_HANDLE) ? &a_filesystem->root_directory : &a_filesystem->open_handles[p_file->handle];

    uint8_t *out_bytes = (uint8_t *)p_out_data;

    // If a file or non-empty directory, modify the byte count to not read beyond the end of the file.
    if (!data->public_fa.is_directory || (data->public_fa.is_directory && data->public_fa.size != 0))
    {
        p_bytes = min(p_bytes, data->public_fa.size - data->public_fa.position);
    }

    // Loop until all required bytes have been read
    while (p_bytes > 0)
    {
        int left_in_sector = SECTOR_SIZE - (data->public_fa.position % SECTOR_SIZE);
        int read = min(p_bytes, left_in_sector);    // Either read all bytes left in the sector or the remaining number of bytes

        // Copy data into output buffer
        memcpy(out_bytes, data->buffer + data->public_fa.position % SECTOR_SIZE, read);
        out_bytes += read;
        data->public_fa.position += read;
        p_bytes -= read;

        // printf("Left in sector %lu, reading byte count %lu\n", left_in_sector, read);

        // At this point, check if there's still more data to read
        if (left_in_sector == read)
        {
            // Handle root directory first as its reading varies
            if (data->public_fa.handle == ROOT_DIRECTORY_HANDLE)
            {
                ++data->current_cluster;

                if (!disk_read_sectors(p_disk, data->current_cluster, 1, &data->buffer))
                {
                    printf("FAT: Reading disk sector %d failed.\n", data->current_cluster);
                    break;
                }
            }
            else
            {
                if (++data->current_sector_in_cluster >= a_filesystem->bs.boot_sector.sectors_per_cluster)
                {
                    data->current_sector_in_cluster = 0;
                    data->current_cluster = fat_next_cluster(data->current_cluster);
                }

                if (data->current_cluster >= 0xff8)
                {
                    data->public_fa.size = data->public_fa.position;
                    break;
                }

                if (!disk_read_sectors(p_disk, fat_cluster_to_lba(data->current_cluster) + data->current_sector_in_cluster, 1, data->buffer))
                {
                    printf("FAT: Reading disk cluster %d failed.\n", data->current_cluster);
                }
                // printf("FAT: Reading disk cluster %d.\n", data->current_cluster);
            }
        }
    }

    return out_bytes - (uint8_t *)p_out_data;
}

bool fat_read_dir_entry(struct DISK *p_disk, struct FAT_File *p_file, struct FAT_DirectoryEntry *p_out_entry)
{
    return fat_read(p_disk, p_file, sizeof(struct FAT_DirectoryEntry), p_out_entry) == sizeof(struct FAT_DirectoryEntry);
}

void fat_close(struct FAT_File *p_file)
{
    if (p_file->handle == ROOT_DIRECTORY_HANDLE)
    {
        p_file->position = 0;
        a_filesystem->root_directory.current_cluster = a_filesystem->root_directory.first_cluster;
    }
    else
    {
        a_filesystem->open_handles[p_file->handle].opened = false;
    }
}
