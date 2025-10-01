#pragma once

#include <allocators>

void kmalloc_init(void);

Allocator kmalloc_allocator(void);

void *kmalloc_alloc(size_t len);

void *kmalloc_calloc(size_t count, size_t size);

void kmalloc_free(void *ptr, size_t len);
