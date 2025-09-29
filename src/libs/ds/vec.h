#pragma once

#include <assert>
#include <result>
#include <stddef.h>
#include <string.h>
#include <traits>

void vec_expand(char *_Nullable *_Nonnull data, size_t *_Nonnull length, size_t *_Nonnull capacity, size_t memsz, Alloc *_Nonnull alloc);

#define VEC(T)                 \
    struct                     \
    {                          \
        T *_Nullable data;     \
        size_t size;           \
        size_t capacity;       \
        Alloc *_Nonnull alloc; \
    }

#define VEC_INIT(SELF, A)               \
    assert((SELF) != NULL);             \
    memset((SELF), 0, sizeof(*(SELF))); \
    (SELF)->alloc = (A)

#define VEC_DEINIT(SELF)               \
    assert((SELF) != NULL);            \
    (SELF)->alloc->free((SELF)->data); \
    memset((SELF), 0, sizeof(*(SELF)));

#define VEC_PUSH(SELF, V)                                                \
    assert((SELF) != NULL);                                              \
    vec_expand((char **)&(SELF)->data, &(SELF)->size, &(SELF)->capacity, \
               sizeof(*(SELF)->data), (SELF)->alloc);                    \
    assert((SELF)->size < (SELF)->capacity);                             \
    (SELF)->data[(SELF)->size++] = (V)

typedef VEC(char) VecChar;
