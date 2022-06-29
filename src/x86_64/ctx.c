#include "ctx.h"

#include <kernel/spinlock.h>

static uint32_t lock = 0;

void context_create(Context *ctx, uintptr_t ip, TaskArgs args)
{
    lock$(lock);

    Regs regs = {};

    regs.cs = (GDT_USER_CODE * 8) | 3;
    regs.ss = (GDT_USER_DATA * 8) | 3;
    regs.rip = ip;
    regs.rsp = USER_STACK_BASE + STACK_SIZE;
    regs.rbp = USER_STACK_BASE;
    regs.rflags = 0x202;

    regs.rdi = args.arg1;
    regs.rsi = args.arg2;
    regs.rdx = args.arg3;
    regs.rcx = args.arg4;
    regs.r8 = args.arg5;

    ctx->regs = regs;

    ctx->syscall_kernel_bstack = (uintptr_t) malloc(STACK_SIZE);
    ctx->syscall_kernel_stack = ctx->syscall_kernel_bstack + STACK_SIZE;

    unlock$(lock);
}

void context_save(Context *ctx, Regs *regs)
{
    ctx->regs = *regs;
}

void context_switch(Context *ctx, Regs *regs)
{
    lock$(lock);
    
    asm_write_msr(MSR_GS_BASE, (uintptr_t) ctx);
    asm_write_msr(MSR_KERN_GS_BASE, (uintptr_t) ctx);

    *regs = ctx->regs;

    unlock$(lock);
}