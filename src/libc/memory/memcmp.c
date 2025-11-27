#include <memory.h>
#include <stdint.h>

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
