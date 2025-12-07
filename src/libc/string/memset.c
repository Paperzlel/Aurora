#include <string.h>
#include <stdint.h>

void *memset(void *dest, int value, int length)
{
    uint8_t *dest_bytes = (uint8_t *)dest;

    for (int i = 0; i < length; i++)
    {
        dest_bytes[i] = value;
    }

    return dest;
}
