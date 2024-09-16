#include <bootstrap.proto>
#include <hello.proto>
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

    IpcPort recv;
    HelloGreeting *greeting = NULL;

    for (;;)
    {
        try$(sys_port_wild(&recv));
        log$("Received port: %d", recv);

        greeting = recv$(recv, HelloGreeting);

        log$("Received greeting: %s", greeting->msg);
    }

    return ok$();
}