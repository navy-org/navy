#pragma once

#include <stdarg.h>
#include <traits>

long fmt(Stream stream, char const fmt[static 1], ...);

long vfmt(Stream stream, char const fmt[static 1], va_list args);
