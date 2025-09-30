#include "e9.h"

long e9_putc(char c)
{
    asm volatile("outb %0, $0xe9" : : "a"(c) : "memory");
    return 1;
}

long e9_puts(size_t n, char const *s)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        e9_putc(s[i]);
    }

    return i;
}
