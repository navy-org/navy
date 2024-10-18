#include <stdio.h>
#include <stdlib.h>
#include <testing>
#include <wrapper.h>

typedef VEC(struct CMUnitTest) TestSuite;
static TestSuite tests;

TEST(sanity_check)
{
    assert_true(1);
}

static __attribute__((constructor(101))) void init_tests(void)
{
    Alloc *alloc = default_alloc();
    VEC_INIT(&tests, alloc);
}

void push_test(struct CMUnitTest test)
{
    VEC_PUSH(&tests, test);
}

int _entry(void)
{
    cmocka_set_message_output(CM_OUTPUT_TAP);
    return _cmocka_run_group_tests("Navy", tests.data, tests.size, NULL, NULL);
}
