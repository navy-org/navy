#pragma once

#include <result>
#include <stdarg.h>
#include <traits>

Res fmt(Stream stream, char const fmt[static 1], ...);
Res vfmt(Stream stream, char const fmt[static 1], va_list args);