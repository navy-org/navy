#pragma once

#include <stdint.h>
#include <traits>

#include "bump.h"

#define FREELIST_MAGIC      0xC15C0BAD
#define FREELIST_FREE_MAGIC 0xB16F00D

typedef struct _FreelistNode
{
    uint32_t magic;
    struct _FreelistNode *next;
} FreelistNode;

typedef struct
{
    BumpAllocator bump;
    size_t granularity;

    FreelistNode *head;
    FreelistNode *tail;
} Freelist;

long freelist_create(Freelist *self, void *memory, size_t sz, size_t granularity);

long freelist_append_region(Freelist *self, void *region, size_t sz);
