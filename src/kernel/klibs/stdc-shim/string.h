#pragma once

#include <stddef.h>

size_t strlen(char const s[static 1]);

void *memcpy(void *restrict dest, void const *restrict src, size_t n);

void *memset(void *dest, int c, size_t n);

int memcmp(void const *s1, void const *s2, size_t n);