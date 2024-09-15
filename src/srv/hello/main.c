#include <bootstrap.proto>
#include <logging>
#include <port>
#include <result>

Res main(int argc, char const **argv)
{
    BootstrapMsg *reply;
    BootstrapMsg msg = {
        .name = "org.navy.hello",
        .type = BOOTSTRAP_REGISTER,
    };

    send$(BOOTSTRAP_PORT, msg);
    reply = recv$(BOOTSTRAP_PORT, BootstrapMsg);

    if (reply->type == BOOTSTRAP_ACK)
    {
        log$("Bootstrap server acked!");
    }

    log$("Argc = %d", argc);
    log$("Hello, World from %s !", argv[0]);

    return ok$();
}