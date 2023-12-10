#include <hal.h>

#include "spinlock.h"

static _Atomic int retain_count = 0;

static void retain_interrupts(void)
{
    retain_count++;
}

static void release_interrupt(void)
{
    retain_count--;
}

void spinlock_acquire(Spinlock spinlock[static 1])
{
    retain_interrupts();
    while (atomic_flag_test_and_set(&spinlock->lock))
    {
        hal_pause();
    }
}

void spinlock_release(Spinlock spinlock[static 1])
{
    atomic_flag_clear(&spinlock->lock);
    release_interrupt();
}