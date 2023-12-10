#pragma once

#include <stdatomic.h>

typedef struct
{
    volatile atomic_flag lock;
} Spinlock;

#define SPINLOCK_INIT \
    (Spinlock) { .lock = ATOMIC_FLAG_INIT }

Spinlock spinlock_new(void);

void spinlock_acquire(Spinlock spinlock[static 1]);

void spinlock_release(Spinlock spinlock[static 1]);