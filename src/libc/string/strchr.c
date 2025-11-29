#include <string.h>

char *strchr(const char *s, int c) {
    while (*s != c && *s != 0) {
        s++;
    }

    return s != NULL ? s : NULL;
}