#pragma once

#include <stddef.h>

#include "loc.h"
#include "utils.h"

#define RES_TYPE(F) \
    F(RES_OK)       \
    F(RES_INVAL)    \
    F(RES_NOMEM)    \
    F(RES_BADALIGN) \
    F(RES_NOENT)

enum res_type
{
    RES_TYPE(make_enum$)
};

static char const *res_type_str[] = {RES_TYPE(make_str$)};

typedef struct
{
    enum res_type type;
    Loc loc;
    union
    {
        size_t uvalue;
        ptrdiff_t ivalue;
    };
} Res;

#define ok$() \
    (Res) { .type = RES_OK, .uvalue = 0, .loc = loc$() }

#define uok$(u) \
    (Res) { .type = RES_OK, .uvalue = (u), .loc = loc$() }

#define iok$(i) \
    (Res) { .type = RES_OK, .ivalue = (i), .loc = loc$() }

#define err$(t) \
    (Res) { .type = (t), .uvalue = 0, .loc = loc$() }

#define try$(EXPR)                   \
    ({                               \
        Res __result = (Res)(EXPR);  \
        if (__result.type != RES_OK) \
            return __result;         \
        __result.ivalue;             \
    })

static inline char const *res_to_str(Res res)
{
    return res_type_str[res.type];
}