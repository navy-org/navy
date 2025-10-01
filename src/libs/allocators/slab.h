#pragma once

#include <stdint.h>
#include <sync>
#include <traits/allocator.h>

#define SLAB_MAGIC 0xD00DC0DE
#define SLAB_FREE  0xACCE55ED

typedef struct _slab
{
    uint32_t magic;
    uintptr_t ptr;
    struct _slab *next;
} Slab;

typedef struct
{
    Allocator base;
    Spinlock lock;
    Allocator *page_alloc;
    size_t size;
    Slab *root;
} SlabAllocator;

long slab_create(SlabAllocator *self, uintptr_t size, Allocator *page_alloc);
