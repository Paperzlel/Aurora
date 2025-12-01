#include <stdio.h>

int sprintf(char *restrict s, const char *restrict format, ...) {
    va_list args;
    
    va_start(args, format);
    int count = vsprintf(s, format, args);
    va_end(args);

    if (count < 0) {
        // TODO: Set errno
    }

    return count;
}
