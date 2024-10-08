#pragma once

#include <result>

typedef struct
{
    Res (*write)(size_t n, char const buf[static n]);
    Res (*read)(size_t n, char buf[static n]);
} Stream;