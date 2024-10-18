#include <fmt>
#include <logger>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* --- Define write as a stream ------------------------------------------- */

static Res stream_write(size_t n, char const buf[static n])
{
    return uok$(write(STDOUT_FILENO, buf, n));
}

static Stream _stream = {
    .write = stream_write,
};

/* --- Define log functions -------------------------------------------------- */

static char const *progName = NULL;

void logging_set_name(char const *name)
{
    progName = (__builtin_strrchr(name, '/') ? __builtin_strrchr(name, '/') + 1 : __FILE__);
}

void _log(LogEvent event, Loc loc, char const *format, ...)
{
    va_list args;
    va_start(args, format);

    if (event != LOG_NONE)
    {
        fmt(_stream, "%s | %s%s\e[0m %s:%d ", progName, level_colors[event], level_names[event], loc.file, loc.line);
    }

    vfmt(_stream, format, args);

    if (event != LOG_NONE)
    {
        _stream.write(1, "\n");
    }

    va_end(args);
}
