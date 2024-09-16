#include <bootstrap.proto>
#include <ds>
#include <logging>
#include <navy>
#include <port>
#include <result>

HashMap(IpcPort, char *, IpcPort);

Res main([[gnu::unused]] int argc, [[gnu::unused]] char **argv)
{
    IpcPort recv, newPort;
    BootstrapMsg *msg;
    BootstrapMsg reply;
    HashMapIpcPort services;

    log$("Hello from bootstrap");
    hashmap_init(&services, hash_cstring_default);

    for (;;)
    {
        try$(sys_port_wild(&recv));
        msg = recv$(recv, BootstrapMsg);

        switch (msg->type)
        {
            case BOOTSTRAP_REGISTER:
            {
                log$("%s wants to register", msg->name);
                try$(hashmap_insert(&services, msg->name, recv));

                reply = (BootstrapMsg){
                    .type = BOOTSTRAP_ACK,
                };

                send$(recv, reply);
                break;
            }

            case BOOTSTRAP_LOOKUP:
            {
                Res port = hashmap_lookup(&services, msg->name);

                if (port.type != RES_OK)
                {
                    reply = (BootstrapMsg){
                        .type = BOOTSTRAP_NOENT,
                    };
                }
                else
                {
                    try$(sys_port_join(recv, port.uvalue, &newPort, IPC_PORT_SEND | IPC_PORT_RECV));
                    reply = (BootstrapMsg){
                        .type = BOOTSTRAP_ACK,
                        .port = newPort,
                    };
                }

                send$(recv, reply);

                break;
            }
            default:
            {
                error$("Unknown message type");
            }
        }
    }

    return ok$();
}