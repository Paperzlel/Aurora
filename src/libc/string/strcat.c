#include <string.h>

char *strcat(char *restrict s1, const char *restrict s2) {
    size_t len = strlen(s1);
    while (*s2) {
        s1[len] = *s2;
        len++;
        s2++;
    }
    
    s1[len] = 0;
    return s1;
}
