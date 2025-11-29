#pragma once

#include <kernel/kdefs.h>

/**
 * File for DMA transfers to and from floppy disks.
 * To avoid cluttering the floppy disk subsystem, we place this specific subsystem here instead.
 */

/**
 * @brief Set the desired address and size of the data to transfer from the floppy disk to the system memory and vice versa. Data is stored in a 
 * 24-byte pattern of PHYSICAL memory - the DMA bus bypasses the CPU as we cannot translate addresses.
 * @param p_address The physical address to transfer data to.
 * @param p_size The size of the memory being used.
 */
void floppy_dma_setup_for_location(void *p_address, uint16_t p_size);

/**
 * @brief Sets the floppy DMA to begin recieving bytes from the disk to the memory. 
 */
void floppy_dma_read();

/**
 * @brief Sets the  desired address to write data from the system memory to the floppy disk. Data is stored in a 24-byte pattern of PHYSICAL memory -
 * the DMA bus bypasses the CPU as we cannot translate addresses.
 * @param p_address The physical address to transfer data from.
 * @param p_size The size of the memory to transfer.
 */
void floppy_dma_write();
