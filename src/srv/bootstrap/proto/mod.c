#include "mod.h"
#include <string.h>

Res bootstrap_register(char const *name)
{
    BootstrapMsg ipc = {
        .type = BOOTSTRAP_REGISTER,
    };
    memcpy(&ipc.name, name, strlen(name));

    send$(BOOTSTRAP_PORT, ipc);
    BootstrapMsg *reply = recv$(BOOTSTRAP_PORT, BootstrapMsg);

    if (reply->type == BOOTSTRAP_ACK)
    {
        return ok$();
    }
    else
    {
        return err$(RES_RESERVED);
    }
}

Res bootstrap_lookup(char const *name)
{
    BootstrapMsg *reply;
    BootstrapMsg ipc = {
        .type = BOOTSTRAP_LOOKUP,
    };

    memcpy(&ipc.name, name, strlen(name));

    for (size_t i = 0; i < BOOTSTRAP_LOOKUP_RETRY; i++)
    {
        send$(BOOTSTRAP_PORT, ipc);
        reply = recv$(BOOTSTRAP_PORT, BootstrapMsg);

        if (reply->type == BOOTSTRAP_ACK)
        {
            return uok$(reply->port);
        }
    }

    return err$(RES_NOSRV);
}
