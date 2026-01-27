#include <aurora/fs/vfs.h>
#include <aurora/memory.h>
#include <aurora/hal/hal.h>

#define AUR_MODULE "fat"
#include <aurora/debug.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


#define MAX_FILE_PATH 256


struct __attribute__((packed)) FAT_EBR12
{
    uint8_t drive_no;                   // Drive number. Don't use.
    uint8_t reserved;                   // Reserved (Windows NT flags)
    uint8_t boot_signature;             // Signature, 0x28 or 0x29
    uint32_t volume_id;                 // "Serial" number for the volume, don't bother with.
    uint8_t volume_label[11];           // Volume label string. Cool, but not really useful.
    uint8_t fat_type_label[8];          // System ID string (i.e. what FAT device is being used). Don't trust.
};

struct __attribute__((packed)) FAT_EBR32
{
    uint32_t sectors_per_fat;           // Size of the FAT in sectors.
    uint16_t flags;                     // Flags. They might even do something!
    uint16_t fat_version_number;        // FAT version number (high byte is major, low byte is minor)
    uint32_t root_dir_cluster_no;       // Cluster number of the root directory.
    uint16_t fsinfo_sector_no;          // Sector number for the FSInfo structure.
    uint16_t backup_bs_sector_no;       // Sector number for the "backup" boot sector (i.e. the BS to call if this one fails).
    uint8_t reserved[12];               // Reserved. Should always be zero when formatted.
    uint8_t drive_no;                   // Drive number. Don't bother trusting it, can change anyways.
    uint8_t nt_flags;                   // Don't use. Windows NT flags.
    uint8_t signature;                  // Signature byte. Must be 0x28 or 0x29.
    uint32_t volume_id_serial;          // "Serial" number for the volume, useful for checking between PCs. Don't bother with.
    char volume_label[11];              // The volume name. Not really useful to us aside from a cool name.
    char system_id[8];                  // Should say "FAT32   ". Don't trust this anyways
};

struct __attribute__((packed)) FAT_BootSector
{
    uint8_t boot_jump[3];               // Jump instruction (usually EB 3C 90)
    uint8_t oem_name[8];                // OEM identifier/name used by the format tool. Usually "MSWIN4.1" but can be "MSDOS5.1" or "mkdosfs ".
    uint16_t bytes_per_sector;          // Number of bytes per sector (usually 512)
    uint8_t sectors_per_cluster;        // Number of sectors per cluster
    uint16_t reserved_sector_count;     // Number of reserved sectors, including boot sectors
    uint8_t fat_table_count;            // Number of FATs on the device. Often 2.
    uint16_t root_dir_entry_count;      // Number of root directory entries
    uint16_t total_sector_count;        // Total sectors on-disk (if zero, then stored in the large total sectors).
    uint8_t media_descriptor_type;      // Type of floppy disk used. Ignore.
    uint16_t sectors_per_fat;           // Number of sectors per FAT. Set only on FAT12/FAT16 drives.
    uint16_t sectors_per_track;         // Number of sectors per track.
    uint16_t head_side_count;           // Number of heads.
    uint32_t hidden_sectors;            // Number of hidden sectors
    uint32_t large_total_sectors;       // Large sector count, set when there are more than 65535 sectors on-disk.

    uint8_t reserved[54];               // Reserved for the EBR, which is different on FAT32 compared with FAT12/16
};

struct __attribute__((packed)) FAT_DirectoryEntry
{
    uint8_t file_name[11];                          // ASCII file name for the file (if the name > 11 bytes, is a LFN). Not NULL terminated.
    uint8_t attribs;                                // Bitmask for file attributes
    uint8_t reserved;                               // Reserved value
    uint8_t creation_time;                          // Length of creation time for the file, in hundreths of a second (between 0-199)
    uint16_t creation_time_hms;                     // Creation date's hour, mintue and second (hour first 5 bits, minute next 6, second last 5 * 2)
    uint16_t creation_time_ymd;                     // Creation date's year, month and day (year first 7 bits, month next 4, day next 5)
    uint16_t last_accessed;                         // Last accessed date. Same format as creation date.
    uint16_t first_cluster_no_high;                 // Higher 16 bits of this entry's first cluster number. In FAT12/FAT16, always 0.
    uint16_t last_modification_time;                // Last modification time. Same format as creation time.
    uint16_t last_modification_date;                // Last modification date. Same format as creation date.
    uint16_t first_cluster_no_low;                  // Lower 16 bits of this entry's first cluster number. Used to find the first cluster for this entry.
    uint32_t size;                                  // Size of the file in bytes.
};

enum FAT_Type
{
    TYPE_FAT12,
    TYPE_FAT16,
    TYPE_FAT32,
    TYPE_EXFAT
};

// Enum that represents the different attributes an entry can have, in a bitmask.
enum FAT_Attributes
{
    // The FAT entry is in read-only mode
    FAT_READ_ONLY = 0x01,
    // The FAT entry is not visible to the regular user
    FAT_HIDDEN =    0x02,
    // The FAT entry is a system entry and may require elevated privelidges to open
    FAT_SYSTEM =    0x04,
    // The FAT entry is a volume index, i.e. it is the name for the drive
    FAT_VOLUME_ID = 0x08,
    // The FAT entry is a directory
    FAT_DIRECTORY = 0x10,
    // The FAT entry is in an archival format
    FAT_ARCHIVE =   0x20,
    // The FAT entry has a long file name (> 11 bytes) and needs to be read accordingly
    FAT_LFN = FAT_READ_ONLY | FAT_HIDDEN | FAT_SYSTEM | FAT_VOLUME_ID
};

struct FAT_DriveConfig
{
    uint8_t drive_id;
    uint8_t type;
    struct FAT_BootSector bs;
};


// No longer limited by floppies reading 512 bytes at a time, enjoy space!
struct FAT_File
{
    uint8_t *data;                  // Buffer of all loaded data from the file.
    bool is_directory;              // Whether the item is a directory or a file
    bool is_root;                   // Whether the FD is the root directory, which is accessed differently.
    uint8_t drive_id;               // Drive ID for whatever device is being accessed.
    uint32_t size;                  // Size of the entry on disk (zero for directories)
    uint32_t loaded_size;           // Amount of memory in the data buffer. Compared with size to see if memory needs to be allocated.
    uint32_t position;              // Position of the file handler relative to the start of the file.
    uint32_t first_cluster;         // Position of the first cluster in memory, relative to the `data_section_lba`.
    uint32_t current_cluster;       // The current cluster that position is pointing to when loading data.
};


struct FAT_Info
{
    uint8_t drive_count;
    struct FAT_DriveConfig **drives;
    uint32_t file_count;
    struct FAT_File *files;

    struct FAT_File root;
    uint32_t data_section_lba;
};


static struct FAT_Info info;
static uint8_t *fat_table = NULL;


/* INTERNAL FUNCTIONS */

// Forward-declare, used by internal functions

uint32_t fat_read_bytes(void *p_handle, uint32_t p_bytes, void **out_buffer);


static uint32_t fat_cluster_to_lba(struct FAT_DriveConfig *p_config, uint32_t p_current_cluster)
{
    return (p_current_cluster - 2) * p_config->bs.sectors_per_cluster + info.data_section_lba;
}


static uint32_t fat_find_next_cluster(struct FAT_DriveConfig *p_config, uint32_t p_current_cluster)
{
    switch (p_config->type)
    {
        case TYPE_FAT12:
        {
            uint32_t ofs = p_current_cluster + (p_current_cluster / 2);
            uint16_t ret = *(uint16_t *)(fat_table + ofs);
            return (p_current_cluster & 1) ? ret >> 4 : ret & 0xfff;
        }
        case TYPE_FAT16:
        {
            uint32_t ofs = p_current_cluster * 2;
            return *(uint16_t *)(fat_table + ofs);
        }
        case TYPE_FAT32:
        case TYPE_EXFAT:
        {
            uint32_t ofs = p_current_cluster * 4;
            uint32_t ret = *(uint32_t *)(fat_table + ofs);
            return (p_config->type == TYPE_FAT32 ? ret & 0x0fffffff : ret & 0xffffffff);
        }
    }

    return 0;
}

static bool fat_is_eof(uint8_t type, uint32_t p_value)
{
    switch (type)
    {
        case TYPE_FAT12:
        {
            return p_value >= 0xff8;
        }
        case TYPE_FAT16:
        {
            return p_value >= 0xfff8;
        }
        case TYPE_FAT32:
        {
            return p_value >= 0x0ffffff8;
        }
        case TYPE_EXFAT:
        {
            return p_value >= 0xfffffff8;
        }
    }

    return false;
}


/**
 * @brief Checks to see if the given directory contains the directory entry pointed to by `p_name`.
 * @param out_entry The entry to output to the user if found
 * @param p_file The "file" (directory) to look in for if the entry exists.
 * @param p_name The name of the directory entry to look for.
 * @return `true` if the entry exists and is owned, `false` if not.
 */
static bool fat_dir_has_entry(struct FAT_DirectoryEntry *out_entry, struct FAT_File *p_file, const char *p_name)
{
    if (!p_file->is_directory) return false;

    char filename[12];
    memset(filename, ' ', 12);
    filename[11] = 0;

    const char *ext = strchr(p_name, '.');
    if (!ext)
    {
        ext = p_name + 11;
    }

    for (int i = 0; i < 8 && p_name[i] && p_name + i < ext; i++)
    {
        filename[i] = toupper(p_name[i]);
    }

    if (ext != p_name + 11)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
        {
            filename[i + 8] = toupper(ext[i + 1]);
        }
    }

    // Allocate directory if it's not yet been written to
    if (!p_file->data)
    {
        bool end_reached = false;
        // Read all directories into a buffer
        while (!end_reached)
        {
            struct FAT_DirectoryEntry entry;
            void *ref_entry = &entry;
            if (!fat_read_bytes(p_file, sizeof(struct FAT_DirectoryEntry), &ref_entry))
            {
                LOG_ERROR("Failed to read bytes from buffer.");
                return false;
            }

            // Null entry, end
            if (!entry.file_name[0])
            {
                end_reached = true;
                break;
            }
        }
    }

    uint8_t *data = p_file->data;

    while (data)
    {
        if (memcmp(filename, data, 11) == 0)
        {
            memcpy(out_entry, data, sizeof(struct FAT_DirectoryEntry));
            return true;
        }

        data += sizeof(struct FAT_DirectoryEntry);
    }

    return false;
}


/**
 * @brief Allocates and sets up a new entry based on the given directory entry. In every case, the data
 * buffer is NULL until required, usually when being read.
 */
static struct FAT_File *fat_file_open_entry(struct FAT_DirectoryEntry *p_entry)
{
    info.files = realloc(info.files, (info.file_count + 1) * sizeof(struct FAT_File));
    info.file_count++;

    struct FAT_File *ret = &info.files[info.file_count - 1];
    ret->first_cluster = (p_entry->first_cluster_no_high << 16) + p_entry->first_cluster_no_low;
    ret->is_directory = p_entry->attribs & FAT_DIRECTORY;
    ret->size = p_entry->size;
    ret->loaded_size = 0;
    ret->data = NULL;
    ret->is_root = false;
    ret->drive_id = 0;
    ret->current_cluster = ret->first_cluster;
    ret->position = 0;
    return ret;
}


/* API DEFINITIONS */


bool fat_initialize(uint8_t p_drive_no, void *p_bootsector)
{
    if (info.drives)
    {
        LOG_WARNING("Additional drives must be registered at mountpoints rather than as the root device.");
        return false;
    }

    info.drives = (struct FAT_DriveConfig **)calloc(1, sizeof(struct FAT_DriveConfig *));
    memset(info.drives, 0, sizeof(struct FAT_DriveConfig *));

    struct FAT_BootSector *bs = (struct FAT_BootSector *)p_bootsector;
    struct FAT_DriveConfig *cfg = malloc(sizeof(struct FAT_DriveConfig));

    cfg->drive_id = p_drive_no;
    int root_dir_sectors = ((bs->root_dir_entry_count << 5) + (bs->bytes_per_sector - 1)) / bs->bytes_per_sector;
    if (bs->bytes_per_sector == 0)
    {
        // Always set to zero on exFAT drives, and has an actual value on any other drive.
        cfg->type = TYPE_EXFAT;
    }
    else if (bs->total_sector_count == 0)
    {
        cfg->type = TYPE_FAT32;
    }
    else
    {
        int total_clusters = (bs->total_sector_count - (bs->reserved_sector_count + (bs->fat_table_count * bs->sectors_per_fat)
                                     + root_dir_sectors)) / bs->sectors_per_cluster;

        if (total_clusters < 4085)
        {
            cfg->type = TYPE_FAT12;
        } 
        else if (total_clusters < 65525)
        {
            cfg->type = TYPE_FAT16;
        }
        else
        {
            LOG_ERROR("Failed to recognise the FAT format of drive %d", p_drive_no);
            return false;
        }
    }

    memcpy(&cfg->bs, bs, sizeof(struct FAT_BootSector));
    info.drives[info.drive_count] = cfg;
    info.drive_count++;

    // struct FAT_EBR12 *ebr12 = (struct FAT_EBR12 *)&bs->reserved;
    struct FAT_EBR32 *ebr32 = (struct FAT_EBR32 *)&bs->reserved;

    int spf = (bs->sectors_per_fat == 0) ? ebr32->sectors_per_fat : bs->sectors_per_fat;

    // Allocate the total FAT table upfront
    fat_table = calloc(spf, bs->bytes_per_sector);
    if (!hal_read_bytes(p_drive_no, bs->reserved_sector_count, pvirtual_to_physical(fat_table), spf * bs->bytes_per_sector))
    {
        LOG_ERROR("Failed to read FAT table into memory.");
        return false;
    }

    // Setup root directory
    info.root.first_cluster = cfg->type == TYPE_FAT32 ? ebr32->root_dir_cluster_no : (spf * bs->fat_table_count) + bs->reserved_sector_count;
    info.root.current_cluster = info.root.first_cluster;
    info.root.is_directory = true;
    info.root.is_root = true;
    info.root.drive_id = p_drive_no;
    // Allocate and load root dir, always needs to be loaded so it's faster to do so here.
    info.root.size = bs->root_dir_entry_count * sizeof(struct FAT_DirectoryEntry);
    info.root.data = calloc(bs->root_dir_entry_count, sizeof(struct FAT_DirectoryEntry));
 
    if (!fat_read_bytes(&info.root, sizeof(struct FAT_DirectoryEntry) * bs->root_dir_entry_count, (void **)&info.root.data))
    {
        LOG_ERROR("Failed to read root directory into memory.");
        return false;
    }
    
    info.data_section_lba = bs->reserved_sector_count + (bs->fat_table_count * spf) + root_dir_sectors;

    return true;
}


void *fat_open(const char *p_file, uint8_t p_drive_id)
{
    char name[MAX_FILE_PATH];

    if (p_file[0] == '/') p_file++;

    struct FAT_File *current = &info.root;

    while (*p_file)
    {
        bool last = false;
        const char *delim = strchr(p_file, '/');

        if (delim)
        {
            memcpy(name, p_file, delim - p_file);
            name[delim - p_file + 1] = 0;
            p_file = delim + 1;
        }
        else
        {
            uint32_t len = strlen(p_file);
            memcpy(name, p_file, len);
            name[len] = 0;
            p_file += len;
            last = true;
        }

        struct FAT_DirectoryEntry entry;
        if (!fat_dir_has_entry(&entry, current, name))
        {
            // Close file

            LOG_ERROR("Could not find/read directory %s.", name);
            return NULL;
        }
        else
        {
            // Close file
            if (!last && !(entry.attribs & FAT_DIRECTORY))
            {
                LOG_ERROR("Entry %s is not a directory.", name);
                return NULL;
            }

            // Open directory
            current = fat_file_open_entry(&entry);
        }
    }

    if (current == NULL)
    {
        return NULL;
    }

    return current;
}


uint32_t fat_read_bytes(void *p_handle, uint32_t p_bytes, void **out_buffer)
{
    struct FAT_File *p_file = (struct FAT_File *)p_handle;

    if (!p_file)
    {
        LOG_ERROR("File pointer is null. Unable to allocate data.");
        return 0;
    }

    uint8_t *bytes = (uint8_t *)(*out_buffer);
    if (!out_buffer)
    {
        LOG_ERROR("Can't read data as the out_buffer pointer was NULL.");
        return 0;
    }

    struct FAT_DriveConfig *cfg = info.drives[p_file->drive_id];            // Improper way of accessing it

    if (!p_file->is_directory || (p_file->is_directory && p_file->size != 0))
    {
        if (p_bytes > p_file->size - p_file->position)
        {
            LOG_WARNING("Number of bytes input (%u) is greater than the number left in the file (%u)", p_bytes, p_file->size - p_file->position);
        }

        p_bytes = AMIN(p_bytes, p_file->size - p_file->position);
    }

    // The number of bytes to read is now either the one input or the number until the EOF.

    // File position is reading loaded data, copy it in and return.
    if (p_file->position + p_bytes <= p_file->loaded_size)
    {
        memcpy(bytes, p_file->data + p_file->position, p_bytes);
        bytes += p_bytes;
        p_file->position += p_bytes;
        return bytes - (uint8_t *)(*out_buffer);
    }

    // File is reading partially or entirely unloaded data.
    
    // Use the same pointer for internal and external data to save space if the buffers are the same
    bool reference_internal = false;
    if (p_file->data == bytes || !bytes)
    {
        reference_internal = true;
    }

    // Reallocate the buffer handle if it has insufficient size.
    // If NULL, then alloc p_bytes, else alloc loaded_size + p_bytes
    p_file->data = realloc(p_file->data, p_file->loaded_size + p_bytes);
    p_file->loaded_size += p_bytes;
    // If using the same pointer, set it here.
    if (reference_internal)
    {
        *out_buffer = p_file->data;
        bytes = p_file->data;
    }

    while (p_bytes > 0)
    {
        // Use either a cluster's worth of bytes or the number of bytes remaining.
        int bytes_per_cluster = cfg->bs.sectors_per_cluster * cfg->bs.bytes_per_sector;
        int left_in_sector = bytes_per_cluster - (p_file->position % bytes_per_cluster);
        // Number of bytes to read = bytes requested + current pos or the number left in the sector
        int read = AMIN(p_bytes + p_file->position, left_in_sector);

        /**
         * - Number of bytes to read needs to be the current position + the number requested, or the number left in the sector.
         * - This is then read to the data at the start of the buffer + the number of clusters passed * BPS, as data has to be
         *   re-read under the FDC. 
         * - Data is then copied to the output buffer, next cluster if found, repeat.
         */

        void *physaddr = pvirtual_to_physical(p_file->data + (p_file->current_cluster - p_file->first_cluster) * bytes_per_cluster);

        if (p_file->is_root)
        {
            // Root directory, read directly rather than via other means
            if (!hal_read_bytes(p_file->drive_id, p_file->current_cluster, physaddr, read))
            {
                LOG_ERROR("Error reading bytes for FAT file.");
                break;
            }
        }
        else
        {
            int lba = fat_cluster_to_lba(cfg, p_file->current_cluster);

            if (!hal_read_bytes(p_file->drive_id, lba, physaddr, read))
            {
                LOG_ERROR("Error reading bytes for FAT file.");
                break;
            }
        }

        // When copying, use either the number of bytes read in the sector or the number of bytes requested.
        int copyable_bytes = AMIN(p_bytes, read);
        if (!reference_internal)
        {
            memcpy(bytes, (p_file->data + p_file->position), copyable_bytes);
        }
        bytes += copyable_bytes;
        // Number of bytes to go = number of bytes read - position in the cluster
        p_bytes -= read - (p_file->position % bytes_per_cluster);
        p_file->position += copyable_bytes;

        if (p_bytes <= 0) break; // Don't bother with the next bit

        p_file->current_cluster = fat_find_next_cluster(cfg, p_file->current_cluster);
        if (fat_is_eof(cfg->type, p_file->current_cluster))
        {
            // EOF
            p_file->size = p_file->position;
            break;
        }
    }

    return bytes - (uint8_t *)(*out_buffer);
}


int fat_get_size(void *p_handle)
{
    if (!p_handle) return 0;
    return ((struct FAT_File *)p_handle)->size;
}


int fat_get_position(void *p_handle)
{
    if (!p_handle) return 0;
    return ((struct FAT_File *)p_handle)->position;
}