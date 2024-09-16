#pragma once

#include <result>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HashMap(T, KT, N)              \
    typedef struct _HashMapNode_##N    \
    {                                  \
        size_t hash;                   \
        T value;                       \
        struct _HashMapNode_##N *next; \
    } HashMapNode_##N;                 \
                                       \
    typedef struct                     \
    {                                  \
        HashMapNode_##N *head;         \
        HashMapNode_##N *tail;         \
        size_t (*hash_fn)(KT);         \
    } HashMap##N;

#define hashmap_init(H, F)    \
    memset(H, 0, sizeof(*H)); \
    (H)->hash_fn = F;

#define hashmap_insert(H, K, V) ({                           \
    size_t hash = (H)->hash_fn(K);                           \
    Res ret = ok$();                                         \
    typeof((H)->head) *node = NULL;                          \
    if ((H)->head == NULL)                                   \
    {                                                        \
        node = &((H)->head);                                 \
    }                                                        \
    else                                                     \
    {                                                        \
        node = &((H)->tail->next);                           \
    }                                                        \
    *node = malloc(sizeof(typeof(**node)));                  \
    if (*node == NULL)                                       \
    {                                                        \
        ret = err$(RES_NOMEM);                               \
    }                                                        \
    else                                                     \
    {                                                        \
        **node = (typeof(**node)){.hash = hash, .value = V}; \
        (H)->tail = *node;                                   \
    }                                                        \
    ret;                                                     \
})

#define hashmap_lookup(H, K) ({         \
    size_t hash = (H)->hash_fn(K);      \
    Res ret = err$(RES_NOENT);          \
    typeof((H)->head) node = (H)->head; \
    while (node != NULL)                \
    {                                   \
        if (node->hash == hash)         \
        {                               \
            ret = uok$(node->value);    \
            break;                      \
        }                               \
        node = node->next;              \
    }                                   \
    ret;                                \
})

size_t hash_cstring_default(char *str);
