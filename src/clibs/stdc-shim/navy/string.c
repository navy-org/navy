#include "string.h"

#include "stdlib.h"

size_t strlen(const char *s)
{
    size_t i;
    for (i = 0; s[i] != '\0'; i++)
        ;
    return i;
}

void *memcpy(void *s1, void const *s2, size_t n)
{
    if (s1 == NULL || s2 == NULL || n < 1)
    {
        return NULL;
    }

    char *_Nonnull sb1 = (char *)s1;
    char const *_Nonnull sb2 = (char const *)s2;

    for (size_t i = 0; i < n; i++)
    {
        sb1[i] = sb2[i];
    }

    return s1;
}

void *memset(void *s, int c, size_t n)
{
    if (s == NULL)
    {
        return NULL;
    }

    char *buf = (char *)s;
    for (size_t i = 0; i < n; i++)
    {
        buf[i] = c;
    }

    return s;
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

int strcmp(char const *s1, char const *s2)
{
    return memcmp(s1, s2, strlen(s1));
}

void *memmove(void *s1, void const *s2, size_t n)
{
    char const *usrc = (char const *)s2;
    char *udest = (char *)s1;

    if (udest < usrc)
    {
        for (size_t i = 0; i < n; i++)
        {
            udest[i] = usrc[i];
        }
    }
    else if (udest > usrc)
    {
        for (size_t i = n; i > 0; i--)
        {
            udest[i - 1] = usrc[i - 1];
        }
    }

    return s1;
}

char *strrchr(const char *s, int c)
{
    size_t length = strlen(s);
    size_t pos = length;

    while (s[pos] != c && pos-- > 0)
        ;

    if (pos == length)
    {
        return NULL;
    }

    return (char *)s + pos;
}

char *strdup(char const *s)
{
    char *ret = calloc(1, strlen(s) + 1);
    memcpy(ret, s, strlen(s));
    return ret;
}