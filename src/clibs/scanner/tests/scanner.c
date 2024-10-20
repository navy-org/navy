#include <ctype.h>
#include <scanner>
#include <testing>

TEST(scan_test)
{
    Bscan scan;
    uint8_t data[5] = {1, 2, 3, 4, 5};

    scanner_init(&scan, data, 5);
    assert_int_equal(scan.size, 5);
    assert_int_equal(scan.index, 0);
    assert_int_equal(scanner_tell(&scan), 0);
    assert_memory_equal(data, scan.data, 5);

    assert_int_equal(scanner_peek(&scan), 1);
    assert_int_equal(scanner_tell(&scan), 0);

    assert_true(scanner_has_next(&scan));

    assert_int_equal(scanner_next(&scan), 2);
    assert_int_equal(scanner_tell(&scan), 1);
    assert_int_equal(scan.index, 1);

    scanner_at(&scan, 3);
    assert_int_equal(scanner_peek(&scan), 4);
    assert_int_equal(scanner_tell(&scan), 3);
    assert_true(scanner_has_next(&scan));

    scanner_rewind(&scan, 1);
    assert_int_equal(scanner_tell(&scan), 2);
    assert_int_equal(scanner_peek(&scan), 3);
    assert_true(scanner_has_next(&scan));

    scanner_skip(&scan, 2);
    assert_int_equal(scanner_tell(&scan), 4);
    assert_int_equal(scanner_peek(&scan), 5);
    assert_false(scanner_has_next(&scan));
}

TEST(scan_failure)
{
    Sscan scanner;
    Sscan *null = NULL;
    expect_assert_failure(scanner_init(&scanner, NULL, 99));
    expect_assert_failure(scanner_init(null, "Hello, world!", 99));
    expect_assert_failure(scanner_init(&scanner, "Hello, world!", 0));
    expect_assert_failure(scanner_init(&scanner, "Hello, world!", -1));

    scanner_init(&scanner, "hello, world!", 13);

    expect_assert_failure(scanner_peek(null));
    expect_assert_failure(scanner_at(null, 0));
    expect_assert_failure(scanner_rewind(null, 0));
    expect_assert_failure(scanner_skip(null, 0));
    expect_assert_failure(scanner_next(null));
    expect_assert_failure(scanner_has_next(null));
    expect_assert_failure(scanner_tell(null));

    expect_assert_failure(scanner_at(&scanner, 99));
    expect_assert_failure(scanner_rewind(&scanner, 99));
    expect_assert_failure(scanner_rewind(&scanner, -1));
    expect_assert_failure(scanner_skip(&scanner, 99));

    scanner_skip(&scanner, 13);
    expect_assert_failure(scanner_next(&scanner));
    expect_assert_failure(scanner_peek(&scanner));
    expect_assert_failure(scanner_skip(&scanner, 1));

    expect_assert_failure(scanner_rewind(&scanner, -2));
    expect_assert_failure(scanner_rewind(&scanner, 69));
}
