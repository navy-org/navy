#pragma once

#include <res.h>
#include <string.h>

typedef struct _Alloc
{
    Res (*realloc)(void *ptr, size_t size);
    Res (*malloc)(size_t size);
    Res (*free)(void *ptr);
    Res (*calloc)(size_t count, size_t size);
} Alloc;

typedef Alloc (*AllocAcquireFn)(void);

static inline Res alloc_default_malloc(Alloc *alloc, size_t size)
{
    return alloc->realloc(NULL, size);
}

static inline Res alloc_default_free(Alloc *alloc, void *ptr)
{
    return alloc->realloc(ptr, 0);
}

static inline Res alloc_default_calloc(Alloc *alloc, size_t count, size_t size)
{
    Res res = alloc->realloc(NULL, count * size);

    if (res.type == RES_OK)
    {
        memset((void *)res.uvalue, 0, count * size);
    }

    return res;
}