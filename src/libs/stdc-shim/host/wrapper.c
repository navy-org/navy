#include <stdlib.h>
#include <wrapper.h>

Res _realloc(void *ptr, size_t size)
{
    void *res = realloc(ptr, size);
    if (res == NULL)
    {
        return err$(RES_NOMEM);
    }

    return uok$(res);
}

Res _malloc(size_t size)
{
    void *res = malloc(size);
    if (res == NULL)
    {
        return err$(RES_NOMEM);
    }

    return uok$(res);
}

Res _free(void *ptr)
{
    free(ptr);
    return ok$();
}

Res _calloc(size_t count, size_t size)
{
    void *res = calloc(count, size);
    if (res == NULL)
    {
        return err$(RES_NOMEM);
    }

    return uok$(res);
}

Alloc *default_alloc(void)
{
    static Alloc alloc = {
        .malloc = _malloc,
        .free = _free,
        .realloc = _realloc,
        .calloc = _calloc,
    };

    return &alloc;
}