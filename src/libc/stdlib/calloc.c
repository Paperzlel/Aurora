#include <stdlib.h>
#include <string.h>

void *calloc(size_t nmemb, size_t size)
{
    void *ret = malloc(size * nmemb);
    if (!ret) return NULL;
    memset(ret, 0, size * nmemb);
    return ret;
}
