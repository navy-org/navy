#include <bootstrap.proto>
#include <hello.proto>
#include <logging>
#include <navy>
#include <port>
#include <result>

#define MSG "What's up? I'm the client."

Res main([[gnu::unused]] int argc, [[gnu::unused]] char const *argv[])
{
    BootstrapMsg *reply;
    bool found = false;

    BootstrapMsg msg = {
        .type = BOOTSTRAP_LOOKUP,
        .name = "org.navy.hello",
    };

    log$("Sup from the client!");

    for (size_t i = 0; i < 3; i++)
    {
        send$(BOOTSTRAP_PORT, msg);
        reply = recv$(BOOTSTRAP_PORT, BootstrapMsg);

        if (reply->type == BOOTSTRAP_ACK)
        {
            found = true;
            break;
        }

        log$("Service not found, retrying...");
    }

    if (found)
    {
        log$("Service found, sending message...");

        HelloGreeting greeting = {
            .msg = MSG,
        };

        send$(reply->port, greeting);
    }
    else
    {
        error$("Service not found, exiting...");
    }

    return ok$();
}