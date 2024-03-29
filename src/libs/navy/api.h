#pragma once

#include <res.h>
#include <stddef.h>
#include <stdint.h>

/* --- Types --------------------------------------------------------------- */

typedef uintptr_t SysArg;

typedef enum : size_t
{
    SYS_LOG,

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
    enum res_type res;

    // s : rax, arg1 : rdi, arg2 : rsi, arg3 : rdx, arg4 : r10, arg5 : r8, arg6 : r9
    __asm__ volatile(
        "syscall"
        : "=a"(res)
        : "a"(s), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
        : "rcx", "r11", "memory");

    return (Res){
        .type = res,
        .uvalue = 0,
        .loc = loc$(),
    };
}

#else
#    error "Unsupported architecture"
#endif

#define __syscall(id, a1, a2, a3, a4, a5, a6, ...) __syscall_impl(id, a1, a2, a3, a4, a5, a6)

#define syscall(...) __syscall(__VA_ARGS__, 0, 0, 0, 0, 0, 0)

Res _syscall_handler(Syscalls no, SysArgs args);

/* --- Syscalls ------------------------------------------------------------- */

static inline Res sys_log(char const *str, size_t len)
{
    return syscall(SYS_LOG, (SysArg)str, (SysArg)len);
}
