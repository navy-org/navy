#include <logger>
#include <string.h>

#include "stack.h"

Stack stack_init(uintptr_t base, size_t len)
{
    Stack stack = {
        .base = base + len,
        .len = len,
        .off = 0,
    };

    return stack;
}

size_t stack_push(Stack *self, void *data, size_t size)
{
    self->base -= size;
    self->off += size;

    memcpy((void *)self->base, data, size);
    return self->off;
}
