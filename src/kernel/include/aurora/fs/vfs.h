#ifndef _AURORA_VFS_H
#define _AURORA_VFS_H

#include <aurora/kdefs.h>

/**
 * @brief Structure that represents a given file on-disk.
 */
struct VFS_Handle
{
    bool        open;               // Whether the file is opened (i.e. data can be read to and from it) or not.
    int         handle;             // The handle to the driver-specific file, as a signed 32-bit integer.
    uint32_t    pos;                // The position of the handle into the file (i.e. what position it last read from)
    uint32_t    size;               // The size of the file in bytes.
};

/**
 * @brief Initializes the VFS to detect what drives exist and what formats to read them in.
 * @return `true` if successful, `false` if not.
 */
bool vfs_initialize();

/**
 * @brief Opens a VFS handle to a given file specified by `p_path`. This path must use UNIX-style diretories.
 * @param p_path The path from the root to the file system.
 * @returns The file handle for the file, or `NULL` on failure.
 */
struct VFS_Handle *vfs_open(const char *p_path);

#endif // _AURORA_VFS_H