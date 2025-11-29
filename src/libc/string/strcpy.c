#include <string.h>

char *strcpy(char *restrict s1, const char *restrict s2) {
    int size = 0;
    while (*s2) {
        s1[size] = *s2;
        s2++;
        size++;
    }
    s1[size] = 0;

    return s1;
}
