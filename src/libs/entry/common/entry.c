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
    main(argc, argv);

    for (;;)
        ;
}