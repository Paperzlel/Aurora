#pragma once

#include "disk.h"

typedef struct {
    uint8_t file_name[11];
    uint8_t attribs;
    uint8_t reserved;
    uint8_t creation_time;
    uint16_t creation_time_hms;
    uint16_t creation_time_ymd;
    uint16_t last_accessed;
    uint16_t first_cluster_no_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster_no_low;
    uint32_t size;
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct {
    int handle;
    bool is_directory;
    uint32_t size;
    uint32_t position;
} FAT_File;

typedef enum {
    FAT_READ_ONLY = 0x01,
    FAT_HIDDEN =    0x02,
    FAT_SYSTEM =    0x04,
    FAT_VOLUME_ID = 0x08,
    FAT_DIRECTORY = 0x10,
    FAT_ARCHIVE =   0x20,
    FAT_LFN = FAT_READ_ONLY | FAT_HIDDEN | FAT_SYSTEM | FAT_VOLUME_ID
} FAT_Attributes;

bool fat_initialize(DISK *p_disk);

FAT_File *fat_open(DISK *p_disk, const char *p_path);

/**
 * @brief Reads a number of bytes from the given FAT file into an array which can be used at a later point.
 * @param p_disk The disk instance to read from
 * @param p_file The file to read data from
 * @param p_bytes The number of bytes of data to read into the buffer
 * @param p_out_data The output pointer to read memory into
 * @returns The number of bytes read from the data into the output buffer
 */
uint32_t fat_read(DISK *p_disk, FAT_File *p_file, uint32_t p_bytes, void *p_out_data);

bool fat_read_dir_entry(DISK *p_disk, FAT_File *p_file, FAT_DirectoryEntry *p_out_entry);

void fat_close(FAT_File *p_file);