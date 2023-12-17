#pragma once

#include <res.h>
#include <stddef.h>
#include <stdint.h>

/* --- MEMMAPS ------------------------------------------------------------- */

#define LOADER_MAX (128)

enum memmap_type
{
    LOADER_FREE,
    LOADER_RESERVED,
    LOADER_RECLAIMABLE,
    LOADER_KERNEL,
    LOADER_FB,
};

typedef struct
{
    size_t base;
    size_t len;
    enum memmap_type type;
} MmapEntry;

typedef struct
{
    size_t len;
    MmapEntry entries[LOADER_MAX];
} Mmap;

typedef struct
{
    size_t phys;
    size_t virt;
} KernelMmap;

Mmap loader_get_mmap(void);

/* --- Modules -------------------------------------------------------------- */

typedef struct
{
    uintptr_t base;
    size_t len;
    char name[64];
} Module;

Module loader_get_module(char const *path);

/* --- Misc ---------------------------------------------------------------- */

KernelMmap loader_get_kernel_mmap(void);