#pragma once

#ifdef __ck_unit_test__
extern void mock_assert(int const result, char const *const expression,
                        char const *const file, int const line);

#    undef assert
#    define assert(expression) \
        mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#elif __ck_host__
#    include <assert.h>
#else
void assert_fail(char const *_Nonnull expr, char const *_Nonnull file, int line, char const *_Nonnull func);
#    define assert(expr) ((expr) ? (void)0 : assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif