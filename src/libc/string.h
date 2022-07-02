#pragma once 

#include <stddef.h>


size_t strlen(char const *s);
char *strrchr(const char *s, int c);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int strncmp(const char *s1, char const *s2, size_t n);