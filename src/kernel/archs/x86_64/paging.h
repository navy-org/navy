#pragma once

#include <stdint.h>

#define PAGE_GET_PHYS(x)            (x & 0x000ffffffffff000)
#define PAGE_GET_FLAGS(x)           (x & 0xfff)
#define PMLX_GET_INDEX(addr, level) (((uint64_t)addr & ((uint64_t)0x1ff << (12 + level * 9))) >> (12 + level * 9))

enum pml_fields : uint64_t
{
    PAGE_PRESENT = 1 << 0,
    PAGE_WRITABLE = 1 << 1,
    PAGE_USER = 1 << 2,
    PAGE_WRITE_THROUGH = 1 << 3,
    PAGE_NO_CACHE = 1 << 4,
    PAGE_ACCESSED = 1 << 5,
    PAGE_DIRTY = 1 << 6,
    PAGE_HUGE = 1 << 7,
    PAGE_GLOBAL = 1 << 8,
    PAGE_NO_EXECUTE = (uint64_t)1 << 63,
};

struct [[gnu::packed]] _page
{
    uintptr_t *_raw;
};

void paging_init(void);
