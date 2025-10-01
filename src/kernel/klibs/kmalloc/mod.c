#include <errno.h>
#include <hal>
#include <logger>
#include <pmm>
#include <string.h>

#include "mod.h"

static SlabAllocator slabs[6] = {0};

void kmalloc_init(void)
{
    Allocator pmm = pmm_allocator();

    if (IS_ERR_VALUE(slab_create(&slabs[0], 8, &pmm)) ||
        IS_ERR_VALUE(slab_create(&slabs[1], 16, &pmm)) ||
        IS_ERR_VALUE(slab_create(&slabs[2], 32, &pmm)) ||
        IS_ERR_VALUE(slab_create(&slabs[3], 64, &pmm)) ||
        IS_ERR_VALUE(slab_create(&slabs[4], 128, &pmm)) ||
        IS_ERR_VALUE(slab_create(&slabs[5], 256, &pmm)))
    {
        error$("failed to create slab allocators");
        hal_panic();
    }
}

static void *_alloc([[gnu::unused]] void *ctx, size_t len)
{
    Allocator *alloc = NULL;

    if (len <= 8)
    {
        alloc = &slabs[0].base;
    }
    else if (len <= 16)
    {
        alloc = &slabs[1].base;
    }
    else if (len <= 32)
    {
        alloc = &slabs[2].base;
    }
    else if (len <= 64)
    {
        alloc = &slabs[3].base;
    }
    else if (len <= 128)
    {
        alloc = &slabs[4].base;
    }
    else if (len <= 256)
    {
        alloc = &slabs[5].base;
    }
    else
    {
        Allocator pmm = pmm_allocator();
        alloc = &pmm;
    }

    return alloc->alloc(alloc, len);
}

static long _free([[gnu::unused]] void *ctx, void *ptr, size_t len)
{
    Allocator *alloc = NULL;

    if (len <= 8)
    {
        alloc = &slabs[0].base;
    }
    else if (len <= 16)
    {
        alloc = &slabs[1].base;
    }
    else if (len <= 32)
    {
        alloc = &slabs[2].base;
    }
    else if (len <= 64)
    {
        alloc = &slabs[3].base;
    }
    else if (len <= 128)
    {
        alloc = &slabs[4].base;
    }
    else if (len <= 256)
    {
        alloc = &slabs[5].base;
    }
    else
    {
        Allocator pmm = pmm_allocator();
        alloc = &pmm;
    }

    return alloc->free(alloc, ptr, len);
}

void *kmalloc_alloc(size_t len)
{
    return _alloc(NULL, len);
}

void kmalloc_free(void *ptr, size_t len)
{
    _free(NULL, ptr, len);
}

void *kmalloc_calloc(size_t count, size_t size)
{
    size_t total = count * size;
    void *ptr = _alloc(NULL, total);

    if (!IS_ERR(ptr))
    {
        memset(ptr, 0, total);
    }

    return ptr;
}

Allocator kmalloc_allocator(void)
{
    return (Allocator){
        .alloc = _alloc,
        .free = _free,
        .realloc = NULL,
    };
}
