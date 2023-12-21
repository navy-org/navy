#pragma once

#include <stdint.h>

#include "regs.h"

#define STACK_SIZE        (kib$(4))
#define KERNEL_STACK_SIZE (0x1000)
#define USER_STACK_TOP    (0x7fffffffe000)
#define USER_STACK_SIZE   (mib$(2))
#define USER_STACK_BASE   (USER_STACK_TOP - USER_STACK_SIZE)
#define USER_HEAP_BASE    (0x80000000000)
#define USER_HEAP_SIZE    (gib$(4))
#define USER_HEAP_TOP     (USER_HEAP_BASE + USER_HEAP_SIZE)

struct _HalContext
{
    uintptr_t syscall_kernel_stack;
    uintptr_t syscall_user_stack;

    struct _HalRegs regs;
    _Alignas(64) uint8_t simd[];
};