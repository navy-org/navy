#pragma once

#include <hal>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uintptr_t base;
    size_t len;
    size_t off;
} Stack;

Stack stack_init(uintptr_t base, size_t len);

size_t stack_push(Stack *self, void *data, size_t size);
