#include "memory.h"

#include <stdint.h>

void memcpy(void *dest, const void *src, int size) {
    for (int i = 0; i < size; i++) {
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
    }
}