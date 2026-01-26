/**
 * - Initialize (post-HAL)
 *  - Define drive types and mountpoints
 *  - Find drive formats
 * - Read:
 *  - Check format type
 *  - Look for file in tree
 *  - Read from file
 * - Write:
 *  - TODO: not implemented, need RTC
 */
#include <aurora/fs/vfs.h>
#include <aurora/hal/hal.h>
#include <aurora/memory.h>

#include <stdlib.h>

#include "fat.h"

#define AUR_MODULE "VFS"
#include <aurora/debug.h>

enum DriveFormat
{
    FORMAT_UNKNOWN,
    FORMAT_FAT,
    FORMAT_EXT,
};


struct VFS_Config
{
    struct VFS_Handle *handles;             // Array of allocated file handles.
    uint32_t allocated_handles;             // Number of file handles currently allocated. 
    bool initialized;                       // Whether the VFS has been initialized or not.
};


static struct VFS_Config cfg = { 0 };


#define VFS_BOOT_SIZE 512
#define MBR_BOOT_SIGNATURE 0xaa55
#define FAT_JMP_INSTRUCTION 0xeb

bool vfs_initialize()
{
    if (cfg.initialized)
    {
        LOG_ERROR("Initialize for vfs_initialize has been called twice. Returning false.");
        return false;
    }
    
    uint8_t drives_to_check = hal_get_drive_count();

    for (int i = 0; i < drives_to_check; i++)
    {
        void *temp_mem = malloc(VFS_BOOT_SIZE);   // Read in BS
        hal_read_bytes(i, 0, (void *)virtual_to_physical((uint32_t)temp_mem), VFS_BOOT_SIZE);

        // 0xAA55 tells us the disk is either an MBR or a FAT file
        if (*((uint16_t *)(temp_mem + 0x1fe)) == MBR_BOOT_SIGNATURE && *((uint8_t *)temp_mem) == FAT_JMP_INSTRUCTION)
        {
            // Pass over to the FAT driver so it can get the details needed
            if (!fat_initialize(i, temp_mem))
            {
                LOG_ERROR("Failed to initialise drive as FAT-formatted.");
                return false;
            }
        }
        else
        {
            // Others, not done yet.
            LOG_WARNING("File format of drive %d is unknown.", i);
        }

        // Done with the memory, free it
        free(temp_mem);
    }

    cfg.initialized = true;
    return true;
}


struct VFS_Handle *vfs_open(const char *p_path)
{
    if (p_path == NULL || !cfg.initialized)
    {
        return NULL;
    }

    // Obtain handle
    void *h = fat_open(p_path, 0);
    if (h == NULL) return NULL;

    // Allocate new handle
    cfg.allocated_handles++;
    cfg.handles = realloc(cfg.handles, sizeof(struct VFS_Handle) * cfg.allocated_handles);
    struct VFS_Handle *ret = &cfg.handles[cfg.allocated_handles - 1];
    ret->handle = (int)h;
    ret->open = true;
    ret->pos = 0;
    ret->size = fat_get_size(h);

    return ret;
}
