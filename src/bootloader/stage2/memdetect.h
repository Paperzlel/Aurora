#pragma once

#include <boot/bootstructs.h>
#include <stdbool.h>

/**
 * @brief Obtains a memory map of the system. A memory map describes regions of physical addresses and whether they are either usable, reserved, or otherwise.
 * @param p_out_map Pointer to the memory map structure we are filling
 * @returns True if the function is successful, and false if something went wrong with the function.
 */
bool memory_get_mem_map(struct MemoryMap *p_out_map);
