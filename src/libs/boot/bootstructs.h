#pragma once

#include <stdint.h>

typedef struct {
    uint64_t base_address, length;
    uint32_t type;
    uint32_t extended_attribs;
} MemoryRegion;

typedef struct {
    int region_count;
    MemoryRegion *regions;
} MemoryMap;

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

typedef struct {
    uint32_t address;

    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    
    uint16_t bytes_per_line;
    uint16_t mode_id;
} VESA_Framebuffer;

typedef struct {
    VESA_Framebuffer framebuffer;
    const char *vendor_name;
    const char *product_name;
} VESA_FramebufferMap;

typedef struct {
    VESA_FramebufferMap framebuffer_map;
    MemoryMap memory_map;
    uint16_t boot_device;
} BootInfo;