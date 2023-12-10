#pragma once

#include <io/stream.h>
#include <res.h>
#include <stdarg.h>

Res fmt(Stream stream, char const fmt[static 1], ...);
Res vfmt(Stream stream, char const fmt[static 1], va_list args);