#include <bootstrap.proto>
#include <logging>
#include <navy>
#include <port>
#include <result>

Res main([[gnu::unused]] int argc, [[gnu::unused]] char **argv)
{
    IpcPort recv;
    BootstrapMsg *msg;
    BootstrapMsg reply;

    log$("Hello from bootstrap");

    for (;;)
    {
        try$(sys_port_wild(&recv));
        msg = recv$(recv, BootstrapMsg);

        if (msg->type == BOOTSTRAP_REGISTER)
        {
            log$("%s wants to register", msg->name);

            reply = (BootstrapMsg){
                .type = BOOTSTRAP_ACK,
            };

            send$(recv, reply);
        }
        else
        {
            error$("Unknown message type");
        }

        break;
    }

    return ok$();
}