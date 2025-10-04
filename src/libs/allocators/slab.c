#include <errno.h>
#include <logger>
#include <unistd.h>
#include <utils.h>

#include "bump.h"
#include "slab.h"

static size_t page_amount(size_t size)
{
    if (size <= 64)
    {
        return 64;
    }
    else if (size <= 128)
    {
        return 32;
    }
    else if (size <= 512)
    {
        return 16;
    }
    else if (size <= 1024)
    {
        return 8;
    }
    else
    {
        return 4;
    }
}

static long slab_replenish(SlabAllocator *self)
{
    size_t n_page = page_amount(self->size + sizeof(Slab));
    void *page = self->page_alloc->alloc(self->page_alloc, n_page * getpagesize());

    if (page == NULL)
    {
        return -ENOMEM;
    }

    size_t sz = align_up$(sizeof(Slab) + self->size, sizeof(void *));
    BumpAllocator bump = bump_allocator_create(page, n_page * getpagesize());

    Slab *root = bump.base.alloc(&bump, sz);
    if (IS_ERR(root))
    {
        return PTR_ERR(root);
    }

    root->ptr = (uintptr_t)root;
    Slab *slab = root;

    while (1)
    {
        void *ptr = bump.base.alloc(&bump, sz);

        if (PTR_ERR(ptr) == -ENOMEM)
        {
            slab->next = NULL;
            break;
        }

        slab->next = ptr;
        slab->next->ptr = (uintptr_t)ptr;
        slab->next->magic = SLAB_MAGIC;
        slab = slab->next;
    }

    if (self->root == NULL)
    {
        self->root = root;
    }
    else
    {
        Slab *last = self->root;

        while (last->next != NULL)
        {
            last = last->next;
        }

        last->next = root;
    }

    return 0;
}

static void *_alloc(void *ctx, [[gnu::unused]] size_t len)
{
    SlabAllocator *alloc = (SlabAllocator *)ctx;

    spinlock_acquire(&alloc->lock);

    if (alloc->root == NULL)
    {
        return ERR_PTR(-EINVAL);
    }

    void *ptr = (void *)(alloc->root->ptr + sizeof(Slab));
    alloc->root = alloc->root->next;

    if (alloc->root == NULL)
    {
        long err = slab_replenish(alloc);
        if (IS_ERR_VALUE(err) || alloc->root == NULL)
        {
            spinlock_release(&alloc->lock);
            return ERR_PTR(err);
        }
    }

    spinlock_release(&alloc->lock);
    return ptr;
}

static long _free(void *ctx, void *ptr, [[gnu::unused]] size_t len)
{
    SlabAllocator *alloc = (SlabAllocator *)ctx;
    spinlock_acquire(&alloc->lock);

    Slab *free = (Slab *)((uintptr_t)ptr - sizeof(Slab));

    if (free->magic != SLAB_MAGIC)
    {
        spinlock_release(&alloc->lock);
        return -EINVAL;
    }

    if (free->ptr + sizeof(Slab) != (uintptr_t)ptr)
    {
        spinlock_release(&alloc->lock);
        return -EINVAL;
    }

    free->next = alloc->root;
    alloc->root = free;
    spinlock_release(&alloc->lock);

    return 0;
}

long slab_create(SlabAllocator *self, uintptr_t size, Allocator *page_alloc)
{
    *self = (SlabAllocator){
        .lock = SPINLOCK_INIT,
        .size = size,
        .base = {
            .alloc = _alloc,
            .free = _free,
            .realloc = NULL,
        },
        .page_alloc = page_alloc,
    };

    return slab_replenish(self);
}
