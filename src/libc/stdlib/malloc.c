#include <stdlib.h>

#if defined(__is_libk)
#include <aurora/memory.h>
#else

#endif

void *malloc(size_t size)
{
#if defined(__is_libk)
    return kalloc(size);
#else
    return NULL;
#endif
}
