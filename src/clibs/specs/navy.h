#pragma once

#include <stddef.h>

typedef enum : size_t
{
    SYS_LOG,
    SYS_ALLOC,
    SYS_DEALLOC,
    SYS_PORT_DEINIT,
    SYS_PORT_SEND,
    SYS_PORT_WILD,
    SYS_PORT_RECV,
    SYS_PORT_JOIN,

    __SYSCALL_LENGTH
} Syscalls;