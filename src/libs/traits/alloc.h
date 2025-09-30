#pragma once

#include <stddef.h>

typedef struct _Alloc
{
    void *(*realloc)(void *ptr, size_t size);
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*calloc)(size_t count, size_t size);
} Alloc;

typedef Alloc (*AllocAcquireFn)(void);
