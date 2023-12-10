#include "string.h"

size_t strlen(char const str[static 1])
{
    size_t len = 0;

    while (*str++)
    {
        len++;
    }

    return len;
}

void *memcpy(void *restrict dst, void const *restrict src, size_t n)
{
    char *restrict d = dst;
    char const *restrict s = src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dst;
}

void *memset(void *dest, int c, size_t n)
{
    char *d = dest;

    while (n--)
    {
        *d++ = c;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (((const char *)s1)[i] != ((const char *)s2)[i])
        {
            return ((const char *)s1)[i] - ((const char *)s2)[i];
        }
    }

    return 0;
}