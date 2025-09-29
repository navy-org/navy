#pragma once

#include <hal>
#include <navy>
#include <result>

typedef struct
{
    const char *name;
    pid_t pid;
    HalPage *space;
    HalContext *ctx;
} Task;

Res task_new(char const *name, Res address_space, Res ip);
