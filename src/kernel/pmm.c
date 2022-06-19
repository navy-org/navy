#include "pmm.h"
#include "const.h"
#include "handover/handover.h"

#include <kernel/spinlock.h>
#include <copland/base.h>

static uint32_t lock = 0;
static Bitmap bitmap = {0};
static size_t last_index = 0;

void pmm_set_used(Range page)
{
    lock$(lock);
    size_t target = page.base / PAGE_SIZE;

    for (size_t i = 0; i < page.length / PAGE_SIZE; i++)
    {
        bitmap_set_bit(&bitmap, target + i);
    }
    unlock$(lock);
}

void pmm_free(Range page)
{
    lock$(lock);
    size_t target = page.base / PAGE_SIZE;

    for (size_t i = 0; i < page.length / PAGE_SIZE; i++)
    {
        bitmap_clear_bit(&bitmap, target + i);
    }
    unlock$(lock);
}

void pmm_init(Handover *handover)
{
    bitmap.length = align_up$(range_end$(
        handover->mmaps[handover->mmap_count-1].range
    ) / (PAGE_SIZE * 8), PAGE_SIZE);

    log$("A bitmap of {M} is needed", bitmap.length);

    for (size_t i = 0; handover->mmap_count; i++)
    {
        MmapEntry *entry = &handover->mmaps[i];

        if ((entry->type == MEMMAP_USABLE || entry->type == MEMMAP_BOOTLOADER_RECLAIMABLE) && entry->range.length >= bitmap.length)
        {
            bitmap.range.base = entry->range.base + handover->hhdm_offset;
            log$("Allocated bitmap at {e}", bitmap.range);

            entry->range.base += bitmap.length;
            entry->range.length -= bitmap.length;

            break;
        }
    }

    if (bitmap.buffer == NULL)
    {
        panic$("Couldn't allocated memory for the PMM Bitmap");
    }

    bitmap_fill(&bitmap, 0xff);

    for (size_t i = 0; i < handover->mmap_count; i++)
    {
        MmapEntry entry = handover->mmaps[i];

        if (entry.type == MEMMAP_USABLE || entry.type == MEMMAP_BOOTLOADER_RECLAIMABLE)
        {
            pmm_free((Range) {
                .base = align_down$(entry.range.base, PAGE_SIZE),
                .length = align_up$(entry.range.length, PAGE_SIZE)
            });
        }
    }

    pmm_set_used((Range) {
        .base = bitmap.range.base - handover->hhdm_offset,
        .length = bitmap.length
    });
}

RangeOption pmm_alloc(size_t size)
{
    Range range = {0};

    for (size_t i = last_index; i < bitmap.length && range.length < size; i++)
    {
        if (!bitmap_is_bit_set(&bitmap, i))
        {
            if (range.length == 0)
            {
                range.base = i * PAGE_SIZE;
            }

            range.length += PAGE_SIZE;
        }
        else 
        {
            range.length = 0;
        }
    }

    if (range.length >= size)
    {
        last_index = (range.base + range.length) / PAGE_SIZE; 
        pmm_set_used(range);
    }
    else 
    {
        if (last_index == 0)
        {
            return NONE(RangeOption);
        }

        last_index = 0;
        return pmm_alloc(size);
    }

    return SOME(RangeOption, range);
}
