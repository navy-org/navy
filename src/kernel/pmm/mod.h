#pragma once

#include <allocators>
#include <stddef.h>

#define PMM_PAGE_SIZE 4096

void pmm_init(void);

void *pmm_alloc_page(void);

long pmm_free_page(void *page);

size_t pmm_available_pages(void);
