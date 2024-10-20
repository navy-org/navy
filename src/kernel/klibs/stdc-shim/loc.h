#pragma once

#include <stddef.h>

typedef struct
{
    char const *file;
    const char *full;
    const char *func;
    size_t line;
} Loc;

#define loc$() ((Loc){     \
    .file = __FILE_NAME__, \
    .full = __FILE__,      \
    .func = __func__,      \
    .line = __LINE__,      \
})