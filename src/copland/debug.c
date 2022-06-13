#include "debug.h"
#include "io.h"

static Writer *debugDevice = NULL;

void define_debug_out(Writer *writer)
{
    debugDevice = writer;
}

void log_impl(char const *filename, size_t line_nbr, char const *format, FmtArgs args)
{
    (void) line_nbr;
    fmt$(debugDevice, "\033[33m{}:{}\033[0m ", filename, line_nbr);
    fmt_impl(debugDevice, format, args);
    debugDevice->putc(debugDevice, '\n');
}