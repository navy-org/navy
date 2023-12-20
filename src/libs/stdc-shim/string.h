#pragma once

#include <stddef.h>

/**
 * @brief The strlen function computes the length of the string pointed to by s.
 * @cite ISO/IEC 9899:2023 - 7.26.6.4 The strlen function
 *
 * @arg s - pointer to the string to be examined
 * @return - the number of characters that precede the terminating null character
 **/

size_t strlen(const char *s);