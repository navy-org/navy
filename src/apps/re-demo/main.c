#include <logger>
#include <result>
#include <scanner>
#include <string.h>

#define test_str "print('Hello, World')"

Res main(void)
{
    Sscan scanner;
    scanner_init(&scanner, test_str, strlen(test_str));

    auto print = re_chain(
        re_word("print"),
        re_single('('));

    auto inside = re_chain(
        re_quote(),
        re_while(re_either(re_alpha(), re_space(), re_single(','))),
        re_quote());

    if (!print.skip(&scanner))
    {
        error$("Not a print");
    }

    char buffer[256] = {0};
    if (inside.match(&scanner, 256, buffer))
    {
        log$("Inside: %s", buffer);
    }

    log$("OK");
    return ok$();
}
