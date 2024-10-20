#include <navy>

#include "stdlib.h"

void *malloc(size_t size)
{
    Res res = default_alloc().malloc(size);
    if (res.type != RES_OK)
    {
        return NULL;
    }

    return (void *)res.uvalue;
}

void free(void *ptr)
{
    default_alloc().free(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
    Res res = default_alloc().calloc(nmemb, size);
    if (res.type != RES_OK)
    {
        return NULL;
    }

    return (void *)res.uvalue;
}
