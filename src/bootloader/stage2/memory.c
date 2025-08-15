#include "memory.h"

#include <stdint.h>

void *memcpy(void *dest, const void *src, int size) {
    uint8_t *p_dest_u8 = (uint8_t *)dest;
    const uint8_t *p_src_u8 = (const uint8_t *)src;

    for (int i = 0; i < size; i++) {
        p_dest_u8[i] = p_src_u8[i];
    }

    return dest;
}

void *memset(void *dest, int value, int length) {
    uint8_t *dest_bytes = (uint8_t *)dest;

    for (int i = 0; i < length; i++) {
        dest_bytes[i] = value;
    }

    return dest;
}

int memcmp(const void *str1, const void *str2, int n) {
    const uint8_t *str1_bytes = (const uint8_t *)str1;
    const uint8_t *str2_bytes = (const uint8_t *)str2;

    for (int i = 0; i < n; i++) {
        int res = str1_bytes[i] - str2_bytes[i];
        if (res != 0) {
            return res;
        }
    }

    return 0;
}

uint32_t segofs_to_linear(uint16_t p_segment, uint16_t p_offset) {
    return ((uint32_t)p_segment << 4) + ((uint32_t)p_offset);
}