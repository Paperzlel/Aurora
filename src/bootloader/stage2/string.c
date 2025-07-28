#include "string.h"

const char *strchr(const char *str, char c) {
    if (!str) {
        return NULL;
    }

    while (*str) {
        if (*str == c) {
            return str;
        }

        ++str;
    }

    return NULL;
}

unsigned strlen(const char *str) {
    unsigned len = 0;

    while (*str) {
        ++len;
        ++str;
    }

    return len;
}