#pragma once

#include <hal>
#include <result>

typedef struct
{
    const char *name;
    size_t pid;
    HalPage *space;
    HalContext *ctx;
} Task;

Res task_new(char const *name, Res address_space, Res ip);
