#include <testing>

#include "cmocka.h"

#undef skip

#include <scanner>
#include <stdio.h>

TEST(RE_SINGLE)
{
    Sscan self;
    char buffer[1];
    ReExpr expr = re_single('a', 'b', 'c');

    scanner_init(&self, "azerty", 6);
    assert_true(expr.skip(&self));

    scanner_rewind(&self, 1);
    assert_int_equal(expr.match(&self, 1, buffer), 1);
    assert_int_equal(buffer[0], 'a');

    scanner_init(&self, "bottle", 6);
    assert_true(expr.skip(&self));

    scanner_rewind(&self, 1);
    assert_int_equal(expr.match(&self, 1, buffer), 1);
    assert_int_equal(buffer[0], 'b');

    scanner_init(&self, "creep", 5);
    assert_true(expr.skip(&self));

    scanner_rewind(&self, 1);
    assert_int_equal(expr.match(&self, 1, buffer), 1);
    assert_int_equal(buffer[0], 'c');

    scanner_init(&self, "abc", 3);
    assert_true(expr.skip(&self));
    assert_int_equal(expr.match(&self, 1, buffer), 1);
    assert_int_equal(buffer[0], 'b');

    char nop[0];
    scanner_rewind(&self, 1);
    assert_int_equal(expr.match(&self, 0, nop), 0);

    expr.free();
}

TEST(RE_CHAIN)
{
    Sscan self;
    char buffer[10];
    ReExpr expr = re_chain(re_single('a'), re_single('b'), re_single('c'));

    scanner_init(&self, "abc", 3);
    assert_true(expr.skip(&self));

    scanner_at(&self, 0);
    assert_int_equal(expr.match(&self, 3, buffer), 3);
    assert_string_equal(buffer, "abc");

    scanner_init(&self, "def", 3);
    assert_false(expr.skip(&self));

    scanner_init(&self, "abd", 3);
    assert_false(expr.skip(&self));

    expr.free();
}

TEST(RE_EITHER)
{
    Sscan self;
    char buffer[10];

    ReExpr expr = re_either(
        re_chain(re_single('a'), re_single('b')),
        re_chain(re_single('c'), re_single('d')));

    scanner_init(&self, "abcd", 4);
    assert_true(expr.skip(&self)); // ab
    assert_true(expr.skip(&self)); // cd

    scanner_at(&self, 0);
    assert_int_equal(expr.match(&self, 10, buffer), 2);
    assert_string_equal(buffer, "ab");

    assert_int_equal(expr.match(&self, 10, buffer), 2);
    assert_string_equal(buffer, "cd");

    scanner_init(&self, "efgh", 4);
    assert_false(expr.skip(&self));

    expr.free();
}

TEST(RE_UNTIL)
{
    Sscan self;
    char buffer[10];

    ReExpr expr = re_until(re_single('_'));
    scanner_init(&self, "after_free", 10);
    assert_true(expr.skip(&self));

    scanner_at(&self, 0);
    assert_int_equal(expr.match(&self, 10, buffer), 5);
    assert_memory_equal(buffer, "after", 5);

    scanner_init(&self, "run&gun", 10);
    assert_false(expr.skip(&self));
}

TEST(RE_WHILE)
{
    Sscan self;
    size_t rewind;
    char buffer[10];

    ReExpr expr = re_while(re_alpha());
    scanner_init(&self, "framework_is_the_foundation", 27);
    assert_true(expr.skip(&self));

    scanner_at(&self, 0);
    assert_int_equal(expr.match(&self, 10, buffer), 9);
    assert_memory_equal(buffer, "framework", 9);

    assert_true(expr.skip(&self));

    assert_int_equal(expr.match(&self, 10, buffer), 0);
    scanner_next(&self);

    rewind = scanner_tell(&self);
    assert_true(expr.skip(&self));
    scanner_at(&self, rewind);

    assert_int_equal(expr.match(&self, 10, buffer), 2);
    assert_memory_equal(buffer, "is", 2);

    scanner_init(&self, "_internalIsTheBarrier", 24);
    assert_true(expr.skip(&self));
    assert_int_equal(expr.match(&self, 10, buffer), 0);

    scanner_next(&self);
    assert_false(expr.skip(&self));

    // assert_false(expr.skip(&self));

    // FP can't you see ?
    // If you could stop being an hypocrite for once,
    // You would see that you are loosing your best engineers.
    // Perhaps are you the one who betray us all ?
    //
    // The one who once were the most loyal to the cause,
    // J̶u̶d̶a̶ Joda
}

TEST(RE_RANGE)
{
    Sscan self;
    char buffer[10];

    ReExpr expr = re_while(re_range('0', '3'));

    scanner_init(&self, "01234", 5);

    assert_true(expr.skip(&self));
    scanner_at(&self, 0);

    assert_int_equal(expr.match(&self, 10, buffer), 4);
    assert_memory_equal(buffer, "0123", 4);
}

TEST(RE_WORD)
{
    Sscan self;
    char buffer[10];

    ReExpr expr = re_word("hello");
    scanner_init(&self, "hello, world!", 13);

    assert_true(expr.skip(&self));
    scanner_at(&self, 0);

    assert_int_equal(expr.match(&self, 10, buffer), 5);
    assert_memory_equal(buffer, "hello", 5);

    assert_int_equal(scanner_peek(&self), ',');
}