#include <fmt>
#include <hal>
#include <logger>
#include <stdarg.h>
#include <sync>

void _log(LogEvent event, Loc loc, char const *format, ...)
{
    hal_disable_interrupts();
    va_list args;
    va_start(args, format);

    if (event != LOG_NONE)
    {
        size_t ms = hal_get_ms();
        size_t seconds = ms / 1000;
        ms %= 1000;

        fmt(hal_dbg_stream(), "[ %d.%D ] kernel | %s%s\e[0m %s:%d ", seconds, ms, level_colors[event], level_names[event], loc.file, loc.line);
    }

    vfmt(hal_dbg_stream(), format, args);

    if (event != LOG_NONE)
    {
        hal_dbg_stream().write(1, "\n");
    }

    va_end(args);
    hal_enable_interrupts();
}
