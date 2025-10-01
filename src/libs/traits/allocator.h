#pragma once

#include <stddef.h>

typedef struct _Alloc
{
    void *(*realloc)(void *ctx, void *ptr, size_t size);
    void *(*alloc)(void *ctx, size_t size);
    long (*free)(void *ctx, void *ptr, size_t size);
    void *(*calloc)(void *ctx, size_t count, size_t size);
} Allocator;

typedef Allocator (*AllocAcquireFn)(void);
