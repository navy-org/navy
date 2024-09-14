#include "mod.h"
#include <logging>
#include <string.h>

Res port_alloc(uint64_t rights)
{
    IpcPort port;

    try$(sys_port_alloc(&port, rights));

    return uok$(port);
}