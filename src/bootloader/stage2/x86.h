#pragma once
#include <stdint.h>
#include <stdbool.h>

#define __cdecl __attribute__((cdecl))

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