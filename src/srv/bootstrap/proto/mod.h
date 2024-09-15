#pragma once

#include <result>

#define BOOTSTRAP_PORT (0)

typedef enum
{
    BOOTSTRAP_REGISTER,
    BOOTSTRAP_ACK
} BootstrapMsgType;

typedef struct [[gnu::packed]]
{
    BootstrapMsgType type;
    union
    {
        char name[64];
    };
} BootstrapMsg;
