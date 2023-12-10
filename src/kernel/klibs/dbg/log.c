#include <fmt/fmt.h>
#include <hal.h>
#include <stdarg.h>
#include <sync/spinlock.h>

#include "log.h"

static Spinlock _lock = SPINLOCK_INIT;

static char const *level_names[LOG_EVENT_LENGTH] = {
    [LOG_NONE] = "",
    [LOG_INFO] = "INFO",
    [LOG_WARN] = "WARN",
    [LOG_ERROR] = "ERROR",
    [LOG_CRIT] = "CRITIC",
};

static char const *level_colors[LOG_EVENT_LENGTH] = {
    [LOG_NONE] = "",
    [LOG_INFO] = "\e[1;34m",
    [LOG_WARN] = "\e[1;33m",
    [LOG_ERROR] = "\e[1;31m",
    [LOG_CRIT] = "\e[1;35m",
};

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