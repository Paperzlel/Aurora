#pragma once

#include <stdint.h>

void *memcpy(void *dest, const void *src, int size);

void *memset(void *dest, int value, int length);

int memcmp(const void *str1, const void *str2, int n);

uint32_t segofs_to_linear(uint16_t p_segment, uint16_t p_offset);