#include <logger>
#include <result>
#include <stdlib.h>

[[gnu::weak]] Res main([[gnu::unused]] int argc, [[gnu::unused]] char **argv)
{
    return ok$();
}

int _entry([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    logging_set_name(argv[0]);

    Res res = main(argc, argv);
    if (res.type != RES_OK)
    {
        error$("main returned error: %s from %s:%d", res_to_str(res), res.loc.full, res.loc.line);
        return 1;
    }

    return 0;
}
