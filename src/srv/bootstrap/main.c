#include <logging>
#include <navy>
#include <result>
#include <string.h>

Res main([[gnu::unused]] int argc, [[gnu::unused]] char **argv)
{
    log$("%d %p", argc, argv[0]);
    return ok$();
}