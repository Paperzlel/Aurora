#include <memory.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, int size) {
    uint8_t *p_dest_u8 = (uint8_t *)dest;
    const uint8_t *p_src_u8 = (const uint8_t *)src;

    for (int i = 0; i < size; i++) {
        p_dest_u8[i] = p_src_u8[i];
    }

    return dest;
}
