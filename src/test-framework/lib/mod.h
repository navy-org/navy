#pragma once

#include <ds>
#include <setjmp.h>
#include <stdarg.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wpadded"

#include <cmocka.h>

#pragma GCC diagnostic pop

void push_test(struct CMUnitTest test);

#define TEST(NAME)                                                  \
    static void NAME(void **state);                                 \
    static __attribute__((constructor(102))) void init_##NAME(void) \
    {                                                               \
        push_test((struct CMUnitTest)cmocka_unit_test(NAME));       \
    }                                                               \
    static void NAME([[maybe_unused]] void **state)
