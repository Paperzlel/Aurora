#include <stdio.h>

#if defined(__is_libk)
#include <aurora/terminal.h>
#else

#endif

void putc(char c)
{
#if defined(__is_libk)
    terminal_write_char(c);
#else
    // Do system calls
#endif
}
