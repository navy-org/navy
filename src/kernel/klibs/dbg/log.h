#pragma once

#include <io/stream.h>

#include "loc.h"

typedef enum
{
    LOG_NONE,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRIT,

    LOG_EVENT_LENGTH
} LogEvent;

#define log$(...)      _log(LOG_INFO, loc$(), __VA_ARGS__)
#define warn$(...)     _log(LOG_WARN, loc$(), __VA_ARGS__)
#define error$(...)    _log(LOG_ERROR, loc$(), __VA_ARGS__)
#define critical$(...) _log(LOG_CRIT, loc$(), __VA_ARGS__)
#define print$(...)    _log(LOG_NONE, (Loc){}, __VA_ARGS__)

void _log(LogEvent event, Loc loc, char const *format, ...);