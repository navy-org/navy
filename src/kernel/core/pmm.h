#pragma once

#include <res.h>
#include <stddef.h>
#include <stdint.h>
#include <utils.h>

#define PMM_PAGE_SIZE (kib$(4))

typedef struct
{
    size_t len;
    size_t last_high;
    size_t last_low;
    uint8_t *bitmap;
} PmmBitmap;

typedef struct
{
    size_t len;
    uintptr_t base;
} PmmObj;

struct pmm_alloc_param
{
    size_t pages;
    bool low;
};

PmmObj _pmm_alloc(size_t pages, struct pmm_alloc_param param);

#define pmm_alloc(pages, ...) _pmm_alloc(pages, (struct pmm_alloc_param){__VA_ARGS__})

Res pmm_init(void);

void pmm_free(PmmObj obj);

size_t pmm_available_pages(void);