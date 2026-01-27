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

/**
 * @brief Closes the VFS handle and cleans up any backend data being used.
 * @param p_handle The corresponding handle to close.
 */
void vfs_close(struct VFS_Handle *p_handle);

/**
 * @brief Reads N bytes from the given file handle into an output buffer, and returns a pointer to said buffer. NOTE: Data in this buffer
 * is referenced and NOT copied, so any modifications must be passed into an allocated copy first.
 * @param p_handle The corresponding file handle. Must be opened.
 * @param p_count The number of bytes to read into the file. Must be between `1` and `EOF`.
 * @return The pointer to the data, or `NULL` if retrieving the data failed.
 */
void *vfs_read(struct VFS_Handle *p_handle, uint32_t p_count);

#endif // _AURORA_VFS_H