
#include <hal.h>
#include <libheap/libheap.h>

#include "kmalloc.h"
#include "../../core/pmm.h"

static void *alloc_block([[gnu::unused]] void *ctx, size_t size)
{
    PmmObj page = pmm_alloc(align_up$(size, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);

    if (page.base == 0)
    {
        return NULL;
    }

    return (void *)hal_mmap_l2h(page.base);
}

static void free_block([[gnu::unused]] void *ctx, void *block, size_t size)
{
    PmmObj page = {
        .base = hal_mmap_h2l((uintptr_t)block),
        .len = align_up$(size, PMM_PAGE_SIZE),
    };

    pmm_free(page);
}

static void hook_log([[gnu::unused]] void *ctx, [[gnu::unused]] enum HeapLogType type,
                     [[gnu::unused]] const char *msg, [[gnu::unused]] va_list args)
{
    return;
}

static struct Heap heap_impl = (struct Heap){
    .alloc = alloc_block,
    .free = free_block,
    .log = hook_log,
};

static Res kmalloc_malloc(size_t size)
{
    void *ptr = heap_alloc(&heap_impl, size);
    return ptr == NULL ? err$(RES_NOMEM) : uok$((uintptr_t)ptr);
}

static Res kmalloc_free(void *ptr)
{
    heap_free(&heap_impl, ptr);
    return ok$();
}

static Res kmalloc_realloc(void *ptr, size_t size)
{
    void *new_ptr = heap_realloc(&heap_impl, ptr, size);
    return new_ptr == NULL ? err$(RES_NOMEM) : uok$((uintptr_t)new_ptr);
}

static Res kmalloc_calloc(size_t nmemb, size_t size)
{
    void *ptr = heap_calloc(&heap_impl, nmemb, size);
    return ptr == NULL ? err$(RES_NOMEM) : uok$((uintptr_t)ptr);
}

Alloc kmalloc_acquire(void)
{
    return (Alloc){
        .malloc = kmalloc_malloc,
        .free = kmalloc_free,
        .realloc = kmalloc_realloc,
        .calloc = kmalloc_calloc,
    };
}