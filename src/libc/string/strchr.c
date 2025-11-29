#include <string.h>

char *strchr(const char *s, int c) {
    char *ptr = (char *)s;
    while (*ptr) {
        if (*ptr == c) {
            return ptr;
        }
        ptr++;
    }

    return NULL;
}