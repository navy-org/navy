#include "alloc.h"
#include <fmt>
#include <libheap/libheap.h>
#include <logging>
#include <navy>

static void *alloc_block([[gnu::unused]] void *ctx, size_t size)
{
    void *ptr;
    Res pages = sys_alloc(&ptr, size);

    if (pages.type != RES_OK)
    {
        return (void *)0xbebebebebebebebe;
    }

    return ptr;
}

static void free_block([[gnu::unused]] void *ctx, void *block, size_t size)
{
    sys_dealloc(block, size);
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

static Res _malloc(size_t size)
{
    void *ptr = heap_alloc(&heap_impl, size);
    return ptr == NULL ? err$(RES_NOMEM) : uok$(ptr);
}

static Res _free(void *ptr)
{
    heap_free(&heap_impl, ptr);
    return ok$();
}

static Res _realloc(void *ptr, size_t size)
{
    void *new_ptr = heap_realloc(&heap_impl, ptr, size);
    return new_ptr == NULL ? err$(RES_NOMEM) : uok$(new_ptr);
}

static Res _calloc(size_t nmemb, size_t size)
{
    void *ptr = heap_calloc(&heap_impl, nmemb, size);
    return ptr == NULL ? err$(RES_NOMEM) : uok$(ptr);
}

Alloc default_alloc(void)
{
    return (Alloc){
        .malloc = _malloc,
        .free = _free,
        .realloc = _realloc,
        .calloc = _calloc,
    };
}