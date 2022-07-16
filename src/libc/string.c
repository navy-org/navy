#include <string.h>

size_t strlen(char const *s)
{
    size_t i = 0;

    if (s == NULL)
    {
        return 0;
    }

    while (s[i++] != '\0');
    return i - 1;
}

void *memset(void *s, int c, size_t n)
{
    if (n)
    {
        char *d = (char *) s;

        while (n--)
        {
            *d++ = c;
        }
    }

    return s;
}

char *strrchr(const char *s, int c)
{
    size_t length = strlen(s);
    size_t pos = length;

    while (s[pos] != c && pos-- > 0);

    if (pos == length)
    {
        return NULL;
    }

    return (char *) s + pos;
}

char *strchr(const char *s, int c)
{
    size_t length = strlen(s);
    size_t pos = 0;
    while (s[pos] != c && pos++ < length);

    if (pos == length)
    {
        return NULL;
    }

    return (char *) s + pos;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    if (src == NULL)
    {
        dest = NULL;
        return NULL;
    }

    size_t i;
    char *cdest = (char *) dest;
    char *csrc = (char *) src;

    for (i = 0; i < n; i++)
    {
        cdest[i] = csrc[i];
    }

    return (void *) cdest;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n--)
    {
        if (*s1++ != *s2++)
        {
            return *s1 - *s2;
        }
    }

    return 0;
}
