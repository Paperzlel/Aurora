#include "floppy_dma.h"

#include <arch/io.h>

/* For the sake of simplicity, we are pretending that DMA channels 4-7 don't exist - we only need channel 2 */

typedef enum {
    DMA_START_1         = 0x02,
    DMA_COUNT_1         = 0x03,
    DMA_START_2         = 0x04,
    DMA_COUNT_2         = 0x05,
    DMA_START_3         = 0x06,
    DMA_COUNT_3         = 0x07,
    DMA_STATUS          = 0x08,
    DMA_COMMAND         = 0x08,
    DMA_REQUEST         = 0x09,
    DMA_CHANNEL_MASK    = 0x0a,
    DMA_MODE            = 0x0b,
    DMA_FLIPFLOP        = 0x0c,
    DMA_INTERMEDIATE    = 0x0d,
    DMA_MASTER_RR       = 0x0d,
    DMA_MASK_RR         = 0x0e,

    DMA_PAGE_ADDR_0     = 0x87,
    DMA_PAGE_ADDR_1     = 0x83,
    DMA_PAGE_ADDR_2     = 0x81,
    DMA_PAGE_ADDR_3     = 0x82,
} DMA_Registers;

typedef enum {
    CHANNEL_MASK_ON     = 1 << 2,
    CHANNEL_SELECT_0    = 0,
    CHANNEL_SELECT_1    = 1 << 0,
    CHANNEL_SELECT_2    = 1 << 1,
    CHANNEL_SELECT_3    = CHANNEL_SELECT_1 | CHANNEL_SELECT_2,

    MODE_SELECT_0                       = CHANNEL_SELECT_0,
    MODE_SELECT_1                       = CHANNEL_SELECT_1,
    MODE_SELECT_2                       = CHANNEL_SELECT_2,
    MODE_SELECT_3                       = CHANNEL_SELECT_3,
    MODE_TRANSFER_SELF_TEST             = 0,
    MODE_TRANSFER_WRITE                 = 1 << 2,           // Writing TO memory
    MODE_TRANSFER_READ                  = 1 << 3,           // Writing FROM memory
    MODE_AUTO                           = 1 << 4,
    MODE_DOWN                           = 1 << 5,
    MODE_MODE_TRANSFER_ON_DEMAND        = 0,
    MODE_MODE_SINGLE_TRANSFER           = 1 << 6,
    MODE_MODE_BLOCK_TRANSFER            = 1 << 7,
    MODE_MODE_CASCADE                   = MODE_MODE_SINGLE_TRANSFER | MODE_MODE_BLOCK_TRANSFER,

    STATUS_TRANSFER_COMPLETE_0  = 1 << 0,
    STATUS_TRANSFER_COMPLETE_1  = 1 << 1,
    STATUS_TRANSFER_COMPLETE_2  = 1 << 2,
    STATUS_TRANSFER_COMPLETE_3  = 1 << 3,
    STATUS_REQUEST_PENDING_0    = 1 << 4,
    STATUS_REQUEST_PENDING_1    = 1 << 5,
    STATUS_REQUEST_PENDING_2    = 1 << 6,
    STATUS_REQUEST_PENDING_3    = 1 << 7,
} DMA_Bits;

void floppy_dma_setup_for_location(void *p_address, uint16_t p_size) {
    p_size -= 1;
    // Mask channels we want to use
    outb(DMA_CHANNEL_MASK, CHANNEL_MASK_ON | CHANNEL_SELECT_2 | CHANNEL_SELECT_0);
    // Reset master flip-flop, can pass any value
    outb(DMA_FLIPFLOP, 0xff);

    // Read address as bytes into the system
    uint32_t address = (uint32_t)p_address;
    outb(DMA_START_2, (uint8_t)(address & 0xff));
    outb(DMA_START_2, (uint8_t)((address >> 8) & 0xff));
    // Reset flip-flop (again)
    outb(DMA_FLIPFLOP, 0xff);
    // Set count - 1
    outb(DMA_COUNT_1, (uint8_t)(p_size & 0xff));
    outb(DMA_COUNT_1, (uint8_t)((p_size >> 8) & 0xff));
    // Set last 24-byte part of address
    outb(DMA_PAGE_ADDR_2, (uint8_t)((address >> 16) & 0xff));

    // Re-mask the channel for use
    outb(DMA_CHANNEL_MASK, CHANNEL_SELECT_2);
}

// NOTE: Should use demand transfer for "advanced" floppy disks

void floppy_dma_read() {
    // Mask channels we want to use
    outb(DMA_CHANNEL_MASK, CHANNEL_MASK_ON | CHANNEL_SELECT_2 | CHANNEL_SELECT_0);
    // Set DMA flags
    outb(DMA_MODE, MODE_MODE_SINGLE_TRANSFER | MODE_AUTO | MODE_TRANSFER_WRITE | MODE_SELECT_2);
    // Re-mask the channel for use
    outb(DMA_CHANNEL_MASK, CHANNEL_SELECT_2);
}

void floppy_dma_write() {
    // Mask channels we want to use
    outb(DMA_CHANNEL_MASK, CHANNEL_MASK_ON | CHANNEL_SELECT_2 | CHANNEL_SELECT_0);
    // Set DMA flags
    outb(DMA_MODE, MODE_MODE_SINGLE_TRANSFER | MODE_AUTO | MODE_TRANSFER_READ | MODE_SELECT_2);
    // Re-mask the channel for use
    outb(DMA_CHANNEL_MASK, CHANNEL_SELECT_2);
}
