#include <stdio.h>

#if defined(__is_libk)
#include <aurora/console.h>
#else

#endif

void putc(char c)
{
#if defined(__is_libk)
    driver_video_write_char(c);
#else
    // Do system calls
#endif
}
