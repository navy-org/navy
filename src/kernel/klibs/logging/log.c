#include <fmt>
#include <hal>
#include <logging>
#include <stdarg.h>
#include <sync>

void _log(LogEvent event, Loc loc, char const *format, ...)
{
    hal_disable_interrupts();
    va_list args;
    va_start(args, format);

    if (event != LOG_NONE)
    {
        fmt(hal_dbg_stream(), "kernel | %s%s\e[0m %s:%d ", level_colors[event], level_names[event], loc.file, loc.line);
    }

    vfmt(hal_dbg_stream(), format, args);

    if (event != LOG_NONE)
    {
        hal_dbg_stream().write(1, "\n");
    }

    va_end(args);
    hal_enable_interrupts();
}