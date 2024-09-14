#include <logging>
#include <navy>
#include <result>
#include <string.h>

#define MSG "What's up? I'm the client."

Res main([[gnu::unused]] int argc, [[gnu::unused]] char const *argv[])
{
    log$("Sup from the client!");
    return ok$();
}