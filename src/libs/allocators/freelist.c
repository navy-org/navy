#include <errno.h>
#include <utils.h>

#include "freelist.h"

long freelist_create(Freelist *self, void *memory, size_t sz, size_t granularity)
{
    if (IS_ERR_OR_NULL(self) || IS_ERR_OR_NULL(memory))
    {
        return -EINVAL;
    }

    BumpAllocator bump = bump_allocator_create(memory, sz);
    self = bump.base.alloc(&bump, sizeof(Freelist));

    if (IS_ERR_OR_NULL(self))
    {
        return PTR_ERR(self);
    }

    self->bump = bump;
    self->granularity = granularity;
    self->head = NULL;
    self->tail = NULL;

    return 0;
}

long freelist_append(Freelist *self, void *item)
{
    if (IS_ERR_OR_NULL(self) || IS_ERR_OR_NULL(item))
    {
        return -EINVAL;
    }

    size_t sz = align_up$(sizeof(FreelistNode) + self->granularity, sizeof(void *));
    FreelistNode *node = self->bump.base.alloc(&self->bump, sz);

    if (IS_ERR_OR_NULL(node))
    {
        return PTR_ERR(node);
    }

    node->magic = FREELIST_MAGIC;
    node->next = NULL;

    if (self->head == NULL)
    {
        self->head = node;
        self->tail = node;
    }
    else
    {
        self->tail->next = node;
        self->tail = node;
    }

    return 0;
}

long freelist_append_region(Freelist *self, void *region, size_t sz)
{
    if (IS_ERR_OR_NULL(self) || IS_ERR_OR_NULL(region) || sz % self->granularity != 0)
    {
        return -EINVAL;
    }

    long err;

    for (size_t offset = 0; offset < sz; offset += self->granularity)
    {
        err = freelist_append(self, (void *)((uintptr_t)region + offset));
        if (IS_ERR_VALUE(err))
        {
            return err;
        }
    }

    return 0;
}
