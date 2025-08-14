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