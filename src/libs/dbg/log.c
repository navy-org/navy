#include <fmt/fmt.h>
#include <hal.h>
#include <navy/api.h>
#include <stdarg.h>

#include "log.h"

/* --- Define sys_log as a stream ------------------------------------------- */

static Res stream_write(size_t n, char const buf[static n])
{
    try$(sys_log(buf, n));
    return uok$(n);
}

static Stream _stream = {
    .write = stream_write,
};

/* --- Define log functions -------------------------------------------------- */

void _log(LogEvent event, Loc loc, char const *format, ...)
{
    va_list args;
    va_start(args, format);

    if (event != LOG_NONE)
    {
        fmt(_stream, "%s%s\e[0m %s:%d ", level_colors[event], level_names[event], loc.file, loc.line);
    }

    vfmt(_stream, format, args);

    if (event != LOG_NONE)
    {
        _stream.write(1, "\n");
    }

    va_end(args);
}