#include "base.h"

#ifdef __osdk_freestanding__
#include <kernel/pmm.h>
#include <kernel/abstraction.h>
#endif


void *__sysdep_morecore(size_t increment)
{
#ifdef __osdk_freestanding__
    RangeOption addr = pmm_alloc(align_up$(increment, PAGE_SIZE) / PAGE_SIZE);
    return addr.succ ? (void *) UNWRAP(addr).base + get_hhdm_offset() : NULL;
#else
#error "Not implemented yet"
#endif
}