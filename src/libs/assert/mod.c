#include <logger>

#include "mod.h"

void assert_fail(char const *_Nonnull expr, char const *_Nonnull file, int line, char const *_Nonnull func)
{
    error$("Assertion failed: {}, file: {}, function: {}, line: {}", expr, file, func, line);

#ifdef __ck_freestanding__
    // panic("Assertion failed");
#endif /* !__ck_freestanding__ */
}