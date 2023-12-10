#include <res.h>

#include "e9.h"

Res e9_putc(char c)
{
    asm volatile("outb %0, $0xe9" : : "a"(c) : "memory");
    return ok$();
}

Res e9_puts(size_t n, char const *s)
{
    for (size_t i = 0; i < n; i++)
    {
        e9_putc(s[i]);
    }

    return ok$();
}