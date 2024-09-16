#pragma once

#include <logging>
#include <result>
#include <stddef.h>
#include <stdint.h>

#include "alloc.h"

/* --- Types --------------------------------------------------------------- */

typedef uintptr_t SysArg;
typedef size_t pid_t;

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

typedef struct
{
    SysArg arg1;
    SysArg arg2;
    SysArg arg3;
    SysArg arg4;
    SysArg arg5;
    SysArg arg6;
} SysArgs;

/* --- Syscall  ------------------------------------------------------------- */

#ifdef __ck_arch_x86_64__

static inline Res __syscall_impl(Syscalls s, SysArg arg1, SysArg arg2, SysArg arg3, SysArg arg4, SysArg arg5, SysArg arg6)
{
    int64_t res;

    // s : rax, arg1 : rdi, arg2 : rsi, arg3 : rdx, arg4 : r10, arg5 : r8, arg6 : r9
    __asm__ volatile(
        "syscall"
        : "=a"(res)
        : "a"(s), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "rcx", "r11", "memory");

    return (Res){
        .type = res,
        .uvalue = 0,
    };
}

#else
#    error "Unsupported architecture"
#endif

#define __syscall(id, a1, a2, a3, a4, a5, a6, ...) __syscall_impl(id, a1, a2, a3, a4, a5, a6)

#define syscall(...) ({                                 \
    Res ret = __syscall(__VA_ARGS__, 0, 0, 0, 0, 0, 0); \
    ret.loc = loc$();                                   \
    ret;                                                \
})

Res _syscall_handler(Syscalls no, SysArgs args);

/* --- Syscalls ------------------------------------------------------------- */

static inline Res sys_log(char const *str, size_t len)
{
    return syscall(SYS_LOG, (SysArg)str, (SysArg)len);
}

static inline Res sys_alloc(void **ptr, size_t size)
{
    return syscall(SYS_ALLOC, (SysArg)ptr, (SysArg)size);
}

static inline Res sys_dealloc(void *ptr, size_t len)
{
    return syscall(SYS_DEALLOC, (SysArg)ptr, len);
}

static inline Res sys_port_join(uintptr_t clientPort, uintptr_t serverPort, uintptr_t *new_port, uint64_t rights)
{
    return syscall(SYS_PORT_JOIN, (SysArg)clientPort, (SysArg)serverPort, (SysArg)new_port, rights);
}

static inline Res sys_port_deinit(uintptr_t *port)
{
    return syscall(SYS_PORT_DEINIT, (SysArg)port);
}

static inline Res sys_port_send(uintptr_t port, void *data, size_t size)
{
    return syscall(SYS_PORT_SEND, (SysArg)port, (SysArg)data, (SysArg)size);
}

static inline Res sys_port_wild(uintptr_t *port)
{
    return syscall(SYS_PORT_WILD, (SysArg)port);
}

static inline Res sys_port_recv(uintptr_t port, void **data)
{
    return syscall(SYS_PORT_RECV, (SysArg)port, (SysArg)data);
}