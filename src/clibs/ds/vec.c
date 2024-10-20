#include "mod.h"

void vec_expand(char *_Nullable *_Nonnull data, size_t *_Nonnull length, size_t *_Nonnull capacity, size_t memsz, Alloc *_Nonnull alloc)
{
    assert(data != NULL && length != NULL && capacity != NULL && alloc != NULL);

    if (*length + 1 > *capacity)
    {
        void *ptr;
        size_t n = (*capacity == 0) ? 1 : *capacity << 1;

        if (*data == nullptr)
        {
            Res alloc_res = alloc->malloc(n * memsz);
            assert(alloc_res.type == RES_OK);
            ptr = (void *)alloc_res.uvalue;
        }
        else
        {
            Res alloc_res = alloc->realloc(*data, n * memsz);
            assert(alloc_res.type == RES_OK);
            ptr = (void *)alloc_res.uvalue;
        }

        assert(ptr != NULL);

        *data = ptr;
        *capacity = n;
    }
}