#pragma once

#include <stdint.h>
#include <sync>
#include <traits>

typedef struct
{
    Allocator base;
    void *heap_start;
    void *heap_end;
    void *next;
    size_t allocations;
    Spinlock lock;
} BumpAllocator;

BumpAllocator bump_allocator_create(void *start, size_t len);
