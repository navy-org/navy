#pragma once 

#include <stddef.h>

void srandom(unsigned int seed);
long random(void);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
