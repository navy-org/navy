#include <ds>
#include <testing>
#include <wrapper.h>

TEST(test_vec_init)
{
    VecChar vec;
    Alloc *alloc = default_alloc();

    VEC_INIT(&vec, alloc);

    assert_null(vec.data);
    assert_int_equal(vec.size, 0);
    assert_int_equal(vec.capacity, 0);
}

TEST(test_vec_deinit)
{
    VecChar vec;
    Alloc *alloc = default_alloc();

    VEC_INIT(&vec, alloc);
    VEC_DEINIT(&vec);

    assert_null(vec.data);
    assert_int_equal(vec.size, 0);
    assert_int_equal(vec.capacity, 0);
}

TEST(test_vec_push)
{
    VecChar vec;
    Alloc *alloc = default_alloc();

    VEC_INIT(&vec, alloc);

    VEC_PUSH(&vec, 'a');
    assert_int_equal(vec.size, 1);
    assert_int_equal(vec.capacity, 1);
    assert_int_equal(vec.data[0], 'a');

    VEC_PUSH(&vec, 'b');
    assert_int_equal(vec.size, 2);
    assert_int_equal(vec.capacity, 2);
    assert_int_equal(vec.data[0], 'a');
    assert_int_equal(vec.data[1], 'b');

    VEC_DEINIT(&vec);
}
