#pragma once

#include <stddef.h>
#include <traits>

void *malloc(size_t size);

void *calloc(size_t nmemb, size_t size);

void free(void *ptr);