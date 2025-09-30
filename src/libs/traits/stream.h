#pragma once

#include <stddef.h>

// FIXME: ssize_t ?
typedef struct
{
    long (*write)(size_t n, char const buf[static n]);
    long (*read)(size_t n, char buf[static n]);
} Stream;
