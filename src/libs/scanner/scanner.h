#pragma once

#include <assert>
#include <stddef.h>
#include <stdint.h>

#define SCANNER(T)        \
    struct                \
    {                     \
        T *_Nonnull data; \
        size_t size;      \
        size_t index;     \
    }

#define scanner_init(SELF, D, SZ)             \
    assert((SELF) != NULL);                   \
    assert((D) != NULL);                      \
    assert((SZ) > 0);                         \
    (SELF)->data = (typeof((SELF)->data))(D); \
    (SELF)->size = SZ;                        \
    (SELF)->index = 0

#define scanner_peek(SELF) ({             \
    assert((SELF) != NULL);               \
    assert((SELF)->index < (SELF)->size); \
    (SELF)->data[(SELF)->index];          \
})

#define scanner_at(self, N) ({   \
    assert((self) != NULL);      \
    assert((N) >= 0);            \
    assert((N) < (self)->size);  \
    (self)->index = (size_t)(N); \
    scanner_peek(self);          \
})

#define scanner_rewind(SELF, N) ({        \
    assert((SELF) != NULL);               \
    assert((SELF)->index >= (size_t)(N)); \
    assert((N) >= 0);                     \
    (SELF)->index -= (size_t)(N);         \
})

#define scanner_skip(SELF, N) ({                 \
    assert((SELF) != NULL);                      \
    assert((SELF)->size - (SELF)->index >= (N)); \
    assert((N) >= 0);                            \
    (SELF)->index += (size_t)(N);                \
    (SELF)->data[(SELF)->index];                 \
})

#define scanner_next(SELF) scanner_skip(SELF, 1)

#define scanner_has_next(SELF) ({     \
    assert((SELF) != NULL);           \
    (SELF)->index + 1 < (SELF)->size; \
})

#define scanner_tell(SELF) ({ \
    assert((SELF) != NULL);   \
    (SELF)->index;            \
})

typedef SCANNER(uint8_t) Bscan;
typedef SCANNER(char) Sscan;
