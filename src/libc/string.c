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
    int *sc = (int *) s;
    for (size_t i = 0; i < n; i++)
    {
        sc[i] = c;
    }

    return (void *) sc;
}