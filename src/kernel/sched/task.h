#pragma once

#include <hal>

typedef struct
{
    const char *name;
    size_t pid;
    HalPage *space;
    HalContext *ctx;
} Task;

Task *task_new(char const *name, HalPage *address_space, uintptr_t ip);

void task_destroy(Task *task);
