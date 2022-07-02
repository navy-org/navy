#include "x86_64/vmm.h"
#include <kernel/spinlock.h>
#include <kernel/pmm.h>
#include <kernel/abstraction.h>

static uint32_t lock;

int liballoc_lock(void)
{
    lock$(lock);
    return 0;
}

int liballoc_unlock(void)
{
    unlock$(lock);
    return 0;
}

void *liballoc_alloc(int pages)
{
    Range addr_range = UNWRAP_OR_PANIC(pmm_alloc(pages * PAGE_SIZE), "Out of memory");
    return (void *) (addr_range.base + get_hhdm_offset());
}

int liballoc_free(void* ptr, int pages)
{
    pmm_free((Range) {
        .base = (uintptr_t) (ptr - get_hhdm_offset()),
        .length = pages * PAGE_SIZE
    });

    return 0;
}