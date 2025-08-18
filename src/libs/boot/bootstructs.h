#pragma once

#include <stdint.h>

// Structure that defines a memory region returned by x86_Memory_GetMemoryRegion().
typedef struct {
    uint64_t base_address;          // The physical base address this memory region begins at
    uint64_t length;                // The size of the memory region in bytes
    uint32_t type;                  // Whether the region is accessible or not (see MmroryRegionType)
    uint32_t extended_attribs;      // Extended optional attributes about the meory region
} MemoryRegion;

// Structure that defines the overall memory map for the system. 
typedef struct {
    int region_count;               // Number of total regions
    MemoryRegion *regions;          // Pointer to an array of said regions
} MemoryMap;

// Enum that describes what type of memory region we have
typedef enum {
    // Region is usable for mapping data to
    MEMORY_REGION_USABLE = 1,
    // Region is being withheld by the BIOS or other structures
    MEMORY_REGION_RESERVED = 2,
    // Region was used by ACPI memory but can be reclaimed (?)
    MEMORY_REGION_ACPI_RECLAIMABLE = 3,
    // Region that are ACPI non-volatile (being used by ACPI)
    MEMORY_REGION_ACPI_NVS = 4,
    // Bad memory; do not use.
    MEMORY_REGION_BAD = 5
} MemoryRegionType;

// Structure that defines information about the current framebuffer being supplied to the kernel.
typedef struct {
    uint32_t address;               // The physical address of the framebuffer

    uint16_t width;                 // The width of the screen
    uint16_t height;                // The height of the screen
    uint8_t bpp;                    // The number of bits per pixel, usually 32 for RGBA8 values
    
    uint16_t bytes_per_line;        // Number of bytes per scan line, may be greater than width * bpp due to padding
    uint16_t mode_id;               // The ID of the VESA video mode, as the bootloader does not enable it by default.
} VESA_Framebuffer;

// Structure that defines the framebuffer map for the system.
typedef struct {
    VESA_Framebuffer framebuffer;           // The "desired" framebuffer, pre-defined in framebuffer.c
    const char *vendor_name;                // The name of the VBE BIOS vendor
    const char *product_name;               // The name of the product running the VBE BIOS
} VESA_FramebufferMap;

// Structure that contains all boot info that is passed from the bootloader to the kernel.
typedef struct {
    VESA_FramebufferMap framebuffer_map;        // Strucure containing the desired framebuffer map
    MemoryMap memory_map;                       // Map of the physical system memory
    uint16_t boot_device;                       // ID of the boot device that is being used
} BootInfo;