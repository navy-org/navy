#pragma once

#include <stddef.h>

#define assert(x) (x ? (void)0 : _ASSERT(__FILE__, __LINE__, #x))

void _ASSERT(char file[static 1], size_t lineno, char expr[static 1]);