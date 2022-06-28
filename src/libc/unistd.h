#pragma once

#include <stdint.h>

typedef enum 
{
    SYS_DEBUG
} SyscallEnum;

#define __syscall_impl2(n, ...) syscall ## n(__VA_ARGS__)
#define __syscall_impl(n, ...) __syscall_impl2(n, __VA_ARGS__)
#define syscall(...) __syscall_impl(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

uint64_t syscall1(uint64_t syscall_id);
uint64_t syscall2(uint64_t syscall_id, uint64_t arg1);
uint64_t syscall3(uint64_t syscall_id, uint64_t arg1, uint64_t arg2);
uint64_t syscall4(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3);
uint64_t syscall5(uint64_t syscall_id, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);;
