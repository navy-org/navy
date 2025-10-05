#include <allocators>
#include <errno.h>
#include <hal>
#include <handover>
#include <logger>
#include <sync>
#include <utils.h>

#include "mod.h"

static _Atomic(size_t) available = 0;
static _Atomic(Freelist *) freelist = NULL;

void pmm_init(void)
{
    HandoverRecord rec;
    bool found_mem = false;

    handover_foreach_record(handover(), rec)
    {
        if (rec.tag == HANDOVER_FREE)
        {
            available += rec.size;
        }
    }

    size_t required_mem = align_up$(sizeof(Freelist) + (sizeof(FreelistNode) * available), PMM_PAGE_SIZE);
    HandoverRecord *recp;

    for (size_t i = 0; i < handover()->count; i++)
    {
        recp = &handover()->records[i];
        if (rec.tag == HANDOVER_FREE && rec.size >= required_mem)
        {
            log$("Freelist will be allocated at %p", recp->start);
            long err = freelist_create(freelist, (void *)hal_mmap_l2h(recp->start), required_mem, PMM_PAGE_SIZE);
            if (IS_ERR_VALUE(err))
            {
                error$("Failed to create physical memory manager: %d", err);
                hal_panic();
            }

            recp->start += required_mem;
            recp->size -= required_mem;

            found_mem = true;
        }
    }

    if (!found_mem)
    {
        error$("Failed to find memory for physical memory manager");
        hal_panic();
    }

    handover_foreach_record(handover(), rec)
    {
        if (rec.tag == HANDOVER_FREE)
        {
            long err = freelist_append_region(freelist, (void *)hal_mmap_l2h(rec.start), rec.size);
            if (IS_ERR_VALUE(err))
            {
                error$("Failed to add free memory region to physical memory manager: %d", err);
                hal_panic();
            }
        }
    }
}

void *pmm_alloc_page(void)
{
    FreelistNode *node = freelist->head;
    freelist->head = node->next;

    if (node == NULL)
    {
        return ERR_PTR(-ENOMEM);
    }

    if (node->magic != FREELIST_MAGIC)
    {
        return ERR_PTR(-EFAULT);
    }

    node->magic = FREELIST_FREE_MAGIC;

    available--;

    return (void *)((uintptr_t)node + sizeof(FreelistNode));
}

long pmm_free_page(void *page)
{
    if (IS_ERR_OR_NULL(page))
    {
        return -EINVAL;
    }

    FreelistNode *node = (FreelistNode *)((uintptr_t)page - sizeof(FreelistNode));

    if (node->magic != FREELIST_FREE_MAGIC)
    {
        return -EFAULT;
    }

    node->magic = FREELIST_FREE_MAGIC;
    node->next = NULL;

    if (freelist->head == NULL)
    {
        freelist->head = node;
        freelist->tail = node;
    }
    else
    {
        freelist->tail->next = node;
        freelist->tail = node;
    }

    available++;

    return 0;
}

size_t pmm_available_pages(void)
{
    return available;
}
