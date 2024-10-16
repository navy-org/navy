#pragma once

#include <port>
#include <result>

#define BOOTSTRAP_PORT (0)
#define BOOTSTRAP_LOOKUP_RETRY (3)

typedef enum
{
    BOOTSTRAP_REGISTER,
    BOOTSTRAP_LOOKUP,
    BOOTSTRAP_ACK,
    BOOTSTRAP_NOENT,
    BOOTSTRAP_ALREAY_REGISTERED,
} BootstrapMsgType;

typedef struct [[gnu::packed]]
{
    BootstrapMsgType type;
    union
    {
        char name[64];
        IpcPort port;
    };
} proto$(BootstrapMsg)

Res bootstrap_register(char const *name);
Res bootstrap_lookup(char const *name);
