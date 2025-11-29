#include <stdlib.h>

#if defined(__is_libk)
#include <kernel/memory.h>
#else

#endif

void free(void *ptr) {
#if defined (__is_libk)
    kfree(ptr);
#else
    
#endif
}