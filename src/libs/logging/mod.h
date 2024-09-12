#pragma once

#include <traits>

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

void _log(LogEvent event, Loc loc, char const *format, ...);