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
bool fat_read_directory(DISK *p_disk, FAT_File *p_current, const char *p_name, FAT_DirectoryEntry *p_directory);