#pragma once

#include <stddef.h>

void simd_init(void);

void simd_context_save(void *ptr);

size_t simd_context_size(void);

void simd_context_init(void *ptr);

void simd_context_load(void *ptr);