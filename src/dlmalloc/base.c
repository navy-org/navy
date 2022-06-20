#include "base.h"

#ifdef KERNEL_MODE
#include <kernel/pmm.h>
#include <kernel/const.h>
#endif


void *__sysdep_morecore(size_t increment)
{
#ifdef KERNEL_MODE
    RangeOption addr = pmm_alloc(align_up$(increment, PAGE_SIZE) / PAGE_SIZE);
    return addr.succ ? (void *) UNWRAP(addr).base + get_hhdm_offset() : NULL;
#else
#error "Not implemented yet"
#endif
}