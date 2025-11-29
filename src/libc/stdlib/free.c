#include <stdlib.h>

#if defined(__is_libk)
#include <kernel/memory.h>
#else

#endif

void free(void *ptr) {
#if defined (__is_libk)
    memfree(ptr);
#else
    
#endif
}