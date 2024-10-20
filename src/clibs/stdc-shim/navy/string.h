#pragma once

#include <stddef.h>

size_t strlen(const char *s);

void *memcpy(void *s1, void const *s2, size_t n);

void *memset(void *s, int c, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(char const *s1, char const *s2);

void *memmove(void *dest, void const *src, size_t n);

char *strrchr(const char *s, int c);

char *strdup(char const *s);
