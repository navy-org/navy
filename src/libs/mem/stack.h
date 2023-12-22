#pragma once

#include <hal.h>
#include <stddef.h>
#include <stdint.h>
#include <tinyvmem/tinyvmem.h>

typedef struct
{
    uintptr_t base;
    size_t len;
    size_t off;
} Stack;

struct stack_push_param
{
    HalPage *vspace;
    Vmem *vmem;
};

Stack stack_init(uintptr_t base, size_t len);

size_t stack_push(Stack *self, void *data, size_t size);
