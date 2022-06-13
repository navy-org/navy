#pragma once 

#include <map-macro>
#include <copland>

#include <stdint.h>
#include <stddef.h>

typedef enum 
{
    FMT_INT, 
    FMT_CHAR,
    FMT_CSTR,
    FMT_STR,
} FmtValueType;

typedef struct 
{
    FmtValueType type;

    union 
    {
        int64_t _int;
        char _char;
        char const *_str;
    };
} FmtValue;

typedef struct 
{
    char const *fmt;
    size_t size;
    size_t offset;
} FmtParser;

static inline FmtValue fmtvali(int64_t val)
{
    return (FmtValue) {
        .type = FMT_INT,
        ._int = val
    };
}

static inline FmtValue fmtvalcs(char const *val)
{
    return (FmtValue) {
        .type = FMT_CSTR,
        ._str = val
    };
}

static inline FmtValue fmtvalc(char val)
{
    return (FmtValue) {
        .type = FMT_CHAR,
        ._char = val
    };
}

#define SELECT_VALUE(__value) _Generic (                \
    (__value),                                          \
    int: fmtvali,                                       \
    char const *: fmtvalcs,                             \
    char *: fmtvalcs,                                   \
    char: fmtvalc                                       \
)(__value),

#define fmt$(w, __format, __arguments...)               \
    fmt_impl(                                           \
        (Writer *) (w),                                 \
        (__format),                                     \
        (FmtValue[]){MAP(SELECT_VALUE, __arguments)},   \
        GET_ARG_COUNT(__arguments)                      \
    )                                                   \

void fmt_impl(Writer *writer, char const *fmt, FmtValue *value, size_t count);