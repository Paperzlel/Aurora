#include <stdio.h>
#include <stdarg.h>

#define OUT_MAX_LEN 1024

int printf(const char *restrict fmt, ...) {
    char msg[OUT_MAX_LEN] =  { 0 };
    va_list arg_ptr;
    
    va_start(arg_ptr, fmt);
    int count = vsprintf(msg, fmt, arg_ptr);
    va_end(arg_ptr);
    
    if (count >= OUT_MAX_LEN) {
#if !defined(__is_libk)
        // Warn of buffer overrun in CRT
#else
    // Do something
#endif
    } else if (count < 0) {
        return count;
    }
    
    puts(msg);
    return count;
}
