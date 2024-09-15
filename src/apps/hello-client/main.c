#include <bootstrap.proto>
#include <logging>
#include <navy>
#include <port>
#include <result>

#define MSG "What's up? I'm the client."

Res main([[gnu::unused]] int argc, [[gnu::unused]] char const *argv[])
{
    BootstrapMsg msg = {
        .type = BOOTSTRAP_LOOKUP,
        .name = "org.navy.hello",
    };

    send$(BOOTSTRAP_PORT, msg);

    log$("Sup from the client!");
    return ok$();
}