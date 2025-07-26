#include "memory.h"

void *memset(void *ptr, int value, uint16_t num)
{
    uint8_t *uptr = (uint8_t *)ptr;
    
    for (uint16_t i = 0; i < num; i++) {
        uptr[i] = (uint8_t)value;
    }

    return ptr;
}