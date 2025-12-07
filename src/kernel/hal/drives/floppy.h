#pragma once

#include <aurora/kdefs.h>

void floppy_initialize();

void *floppy_read(uint8_t p_drive, uint16_t p_lba, void *p_to, size_t p_size);

bool floppy_write(uint8_t p_drive, uint16_t p_lba, void *p_from, size_t p_size);
