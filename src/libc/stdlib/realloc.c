#include <stdlib.h>

#if defined(__is_libk)
#include <aurora/memory.h>
#else

#endif

void *realloc(void *ptr, size_t size)
{
#if defined(__is_libk)
    return krealloc(ptr, size);
#else
    return NULL;
#endif
}
