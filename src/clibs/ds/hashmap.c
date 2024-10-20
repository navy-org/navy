#include <math.h>

#include "mod.h"

#ifndef __ck_sys_kernel__

size_t hash_cstring_default(char *str)
{
    size_t h = 0;
    size_t p = 53;
    size_t m = 1e9 + 9;

    for (size_t i = 1; str[i]; i++)
    {
        h += (str[i] * (size_t)((pow(p, i))));
    }

    return h % m;
}

#endif