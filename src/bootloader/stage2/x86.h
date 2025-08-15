#pragma once
#include <stdint.h>
#include <stdbool.h>

#define __cdecl __attribute__((cdecl))

void __cdecl x86_outb(uint16_t p_port, uint8_t p_data);

uint8_t __cdecl x86_inb(uint16_t p_port);

bool __cdecl x86_Drive_GetDriveParameters(uint8_t p_drive_no, 
            uint8_t *out_drive_type, 
            uint16_t *out_cylinder_count, 
            uint16_t *out_sector_count,
            uint16_t *out_head_count);

bool __cdecl x86_Drive_ReadDisk(uint8_t p_sector_count, 
            uint8_t p_drive_no, 
            uint8_t p_cylinder, 
            uint8_t p_sector, 
            uint8_t p_head, 
            void *out_data);

bool __cdecl x86_Drive_ResetDisk(uint8_t p_drive_no);

uint8_t __cdecl x86_Memory_GetMemoryRegion(uint16_t p_region,
            uint16_t *p_out_region,
            void *p_out_data);

bool __cdecl x86_VBE_GetVESAInfo(void *p_struct, uint8_t *p_is_supported);