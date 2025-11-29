#include <stdlib.h>

#if defined(__is_libk)
#include <stdio.h>
#else

#endif

__attribute__((__noreturn__))
void abort() {
#if defined(__is_libk)
    printf("Fatal error has occured, aborting...\n");
    __asm__("cli");
    __asm__("hlt");
#else
    
#endif
}