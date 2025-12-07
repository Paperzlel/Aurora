#pragma once

#include "disk.h"

// Packed data structure that represents the layout of data in a FAT12 directory entry.
struct __attribute__((packed)) FAT_DirectoryEntry {
    uint8_t file_name[11];                          // ASCII file name for the file (if the name > 11 bytes, is a LFN). Not NULL terminated.
    uint8_t attribs;                                // Bitmask for file attributes
    uint8_t reserved;                               // Reserved value
    uint8_t creation_time;                          // Length of creation time for the file, in hundreths of a second (between 0-199)
    uint16_t creation_time_hms;                     // Creation date's hour, mintue and second (hour first 5 bits, minute next 6, second last 5 * 2)
    uint16_t creation_time_ymd;                     // Creation date's year, month and day (year first 7 bits, month next 4, day next 5)
    uint16_t last_accessed;                         // Last accessed dat. Same format as creation date.
    uint16_t first_cluster_no_high;                 // Higher 16 bits of this entry's first cluster number. In FAT12/FAT16, always 0.
    uint16_t last_modification_time;                // Last modification time. Same format as creation time.
    uint16_t last_modification_date;                // Last modification date. Same format as creation date.
    uint16_t first_cluster_no_low;                  // Lower 16 bits of this entry's first cluster number. Used to find the first cluster for this entry.
    uint32_t size;                                  // Size of the file in bytes.
};

// Structure that represents an entry in the FAT filesystem. 
struct FAT_File {
    int handle;                 // Handle to the internal data of this entry.
    bool is_directory;          // Says whether the entry is a directory or a file.
    uint32_t size;              // The size of the entry in bytes; 0 for a directory.
    uint32_t position;          // The offset from the initial position of the entry in memory.
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

/**
 * @brief Initializes the FAT file system, by loading the boot sector,FAT (File Allocation Table) and root directory into memory.
 * @param p_disk The drive to load the FAT information from. WARNING: This data may not be in the FAT format.
 * @returns True if the operation succeeded, and false if something failed otherwise.
 */
bool fat_initialize(struct DISK *p_disk);

/**
 * @brief Looks for and opens the FAT entry found at the given path. This function uses UNIX-style directories.
 * @param p_disk The drive to load information from
 * @param p_path The path to the file one wants to load
 * @returns A file handle if successful, and NULL if not.
 */
struct FAT_File *fat_open(struct DISK *p_disk, const char *p_path);

/**
 * @brief Reads a number of bytes from the given FAT file into an array which can be used at a later point.
 * @param p_disk The disk to read from
 * @param p_file The file to read data from
 * @param p_bytes The number of bytes of data to read into the buffer
 * @param p_out_data The output buffer to read memory into
 * @returns The number of bytes read from the data into the output buffer
 */
uint32_t fat_read(struct DISK *p_disk, struct FAT_File *p_file, uint32_t p_bytes, void *p_out_data);

/**
 * @brief Reads a FAT directory entry structure from the given disk. 
 * @param p_disk The drive to read information from
 * @param p_file The file handle to read the directory information of
 * @param p_out_entry The output directory entry to read data into
 * @returns True if successful, and false if not.
 */
bool fat_read_dir_entry(struct DISK *p_disk, struct FAT_File *p_file, struct FAT_DirectoryEntry *p_out_entry);

/**
 * @brief Closes a file handle, allowing the specific handle ID and its memory to be reused.
 * @param p_file The file handle to close
 */
void fat_close(struct FAT_File *p_file);
