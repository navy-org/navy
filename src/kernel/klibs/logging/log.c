#include <fmt>
#include <hal>
#include <logging>
#include <stdarg.h>
#include <sync>

static Spinlock _lock = SPINLOCK_INIT;

void _log(LogEvent event, Loc loc, char const *format, ...)
{
    spinlock_acquire(&_lock);
    va_list args;
    va_start(args, format);

    if (event != LOG_NONE)
    {
        fmt(hal_dbg_stream(), "%s%s\e[0m %s:%d ", level_colors[event], level_names[event], loc.file, loc.line);
    }

    vfmt(hal_dbg_stream(), format, args);

    if (event != LOG_NONE)
    {
        hal_dbg_stream().write(1, "\n");
    }

    va_end(args);
    spinlock_release(&_lock);
}