#pragma once

#include <result>

typedef struct _Alloc
{
    Res (*realloc)(void *ptr, size_t size);
    Res (*malloc)(size_t size);
    Res (*free)(void *ptr);
    Res (*calloc)(size_t count, size_t size);
} Alloc;

typedef Alloc (*AllocAcquireFn)(void);