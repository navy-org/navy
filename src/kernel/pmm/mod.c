#include <hal>
#include <handover>
#include <logging>
#include <string.h>
#include <sync>

#include "mod.h"

static PmmBitmap bitmap = {0};
static _Atomic(size_t) available = 0;
static bool try_again = false;
static Spinlock lock = SPINLOCK_INIT;

static bool bitmap_is_bit_set(size_t bit)
{
    return bitmap.bitmap[bit / 8] & (1 << (bit % 8));
}

static void pmm_mark_free(uintptr_t base, size_t len)
{
    size_t start = align_up$(base, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
    size_t end = align_down$(base + len, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;

    for (size_t i = start; i < end; i++)
    {
        bitmap.bitmap[i / 8] &= ~(1 << (i % 8));
    }

    available += len / PMM_PAGE_SIZE;
}

static void pmm_mark_used(uintptr_t base, size_t len)
{
    size_t start = align_up$(base, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;
    size_t end = align_down$(base + len, PMM_PAGE_SIZE) / PMM_PAGE_SIZE;

    for (size_t i = start; i < end; i++)
    {
        bitmap.bitmap[i / 8] |= 1 << (i % 8);
    }

    available -= len / PMM_PAGE_SIZE;
}

Res pmm_init(void)
{
    HandoverPayload *hand = handover();
    HandoverRecord last_entry;
    HandoverRecord record;

    for (size_t i = hand->count; i; i--)
    {
        if (hand->records[i - 1].tag != HANDOVER_FILE)
        {
            last_entry = hand->records[i - 1];
            break;
        }
    }

    bitmap.len = align_up$((last_entry.start + last_entry.size) / (PMM_PAGE_SIZE * 8), PMM_PAGE_SIZE);
    bitmap.last_high = bitmap.len - 1;
    log$("Bitmap size: %d bytes", bitmap.len);

    handover_foreach_record(hand, record)
    {
        if (record.tag == HANDOVER_FREE && record.size >= bitmap.len)
        {
            log$("Bitmap base: %p", record.start);
            bitmap.bitmap = (uint8_t *)hal_mmap_l2h(record.start);
            record.start += bitmap.len;
            record.size -= bitmap.len;
            break;
        }
    }

    if (bitmap.bitmap == NULL)
    {
        return err$(RES_NOMEM);
    }

    memset(bitmap.bitmap, 0xFF, bitmap.len);

    handover_foreach_record(hand, record)
    {
        if (record.tag == HANDOVER_FREE)
        {
            pmm_mark_free(record.start, record.size);
        }
    }

    return ok$();
}

PmmObj _pmm_alloc(size_t pages, struct pmm_alloc_param param)
{
    spinlock_acquire(&lock);

    size_t *start = !param.low ? &bitmap.last_low : &bitmap.last_high;
    size_t end = !param.low ? bitmap.len : 0;
    size_t size = 0;
    size_t base = 0;

    while (*start < end)
    {
        if (!bitmap_is_bit_set(!param.low ? (*start)++ : (*start)--))
        {
            if (++size == pages)
            {
                base = *start - pages;
                pmm_mark_used(base, pages);
                try_again = false;

                spinlock_release(&lock);
                return (PmmObj){.base = base * PMM_PAGE_SIZE, .len = pages * PMM_PAGE_SIZE};
            }
        }
        else
        {
            size = 0;
        }
    }

    spinlock_release(&lock);

    if (!try_again)
    {
        warn$("End of the bitmap reached, trying again");
        try_again = true;
        PmmObj obj = _pmm_alloc(pages, param);
        return obj;
    }
    else
    {
        error$("Out of physical memory");
        hal_panic();
    }

    __builtin_unreachable();
}

void pmm_free(PmmObj obj)
{
    spinlock_acquire(&lock);
    pmm_mark_free(obj.base, obj.len);
    spinlock_release(&lock);
}

size_t pmm_available_pages(void)
{
    return available;
}