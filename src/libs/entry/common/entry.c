#include <logging>
#include <result>
#include <stddef.h>
#include <stdint.h>

[[gnu::weak]] Res main([[gnu::unused]] int argc, [[gnu::unused]] char **argv)
{
    return ok$();
}

static void parse_stack(uintptr_t *stack, int *argc, char ***argv)
{
    uintptr_t *sp = stack;
    *argc = *sp++;
    *argv = (char **)(sp);
    sp += *argc;
    sp++;
}

int _entry(char *stack)
{
    int argc = 0;
    char **argv = NULL;

    parse_stack((uintptr_t *)stack, &argc, &argv);

    logging_set_name(argv[0]);

    Res res = main(argc, argv);
    if (res.type != RES_OK)
    {
        error$("main returned error: %s from %s:%d", res_to_str(res), res.loc.full, res.loc.line);
    }

    for (;;)
        ;
}