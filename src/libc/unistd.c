#include "unistd.h"

uint64_t syscall1(uint64_t syscall_id)
{
    return syscall5(syscall_id, 0, 0, 0, 0);
}

uint64_t syscall2(uint64_t syscall_id, uint64_t arg1)
{
    return syscall5(syscall_id, arg1, 0, 0, 0);
}

uint64_t syscall3(uint64_t syscall_id, uint64_t arg1, uint64_t arg2)
{
    return syscall5(syscall_id, arg1, arg2, 0, 0);
}

uint64_t syscall4(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    return syscall5(syscall_id, arg1, arg2, arg3, 0);
}

uint64_t syscall5(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    uint64_t syscall_return;

    asm volatile(
        "syscall \n"
        : "=a"(syscall_return)
        : "a"(syscall_id), "b"(arg1), "d"(arg2), "S"(arg3), "D"(arg4)
        : "memory");

    return syscall_return;
}