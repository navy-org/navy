#include "assert.h"
#include <logging>

void _ASSERT(char file[static 1], size_t lineno, char expr[static 1])
{
    error$("Assertion failed: %s:%d: %s", file, lineno, expr);
    for (;;)
        ;
}