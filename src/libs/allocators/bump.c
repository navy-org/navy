#include <errno.h>
#include <logger>
#include <utils.h>

#include "bump.h"

static void *alloc(void *ctx, size_t len)
{
    size_t aligned_len = len & 7 ? align_up$(len, 8) : len;

    BumpAllocator *self = (BumpAllocator *)ctx;
    uint8_t *ptr = self->next;
    self->next = (void *)((uint8_t *)self->next + aligned_len);

    if ((uintptr_t)self->next > (uintptr_t)self->heap_end)
    {
        return ERR_PTR(-ENOMEM);
    }

    self->allocations++;
    return (void *)ptr;
}

static long free(void *ctx, [[gnu::unused]] void *ptr, [[gnu::unused]] size_t len)
{
    BumpAllocator *self = (BumpAllocator *)ctx;
    self->allocations--;

    if (self->allocations == 0)
    {
        self->next = self->heap_start;
    }

    return 0;
}

BumpAllocator bump_allocator_create(void *start, size_t len)
{
    return (BumpAllocator){
        .base = {
            .alloc = alloc,
            .free = free,
            .realloc = NULL,
        },
        .heap_start = start,
        .heap_end = (char *)start + len,
        .next = start,
        .allocations = 0,
    };
}
