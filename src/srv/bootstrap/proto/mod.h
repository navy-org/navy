#pragma once

#include <port>
#include <result>

#define BOOTSTRAP_PORT (0)

typedef enum
{
    BOOTSTRAP_REGISTER,
    BOOTSTRAP_LOOKUP,
    BOOTSTRAP_ACK,
    BOOTSTRAP_NOENT,
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
