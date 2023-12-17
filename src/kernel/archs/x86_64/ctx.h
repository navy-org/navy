#pragma once

#include <stdint.h>

#include "regs.h"

#define STACK_SIZE        (kib$(4))
#define KERNEL_STACK_SIZE (0x1000)
#define USER_STACK_BASE   (0xff0000000)

struct _HalContext
{
    uintptr_t syscall_kernel_stack;
    uintptr_t syscall_user_stack;

    struct _HalRegs regs;
    _Alignas(64) uint8_t simd[];
};