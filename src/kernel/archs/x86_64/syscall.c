#include <hal>
#include <logger>

#include "asm.h"
#include "gdt.h"
#include "syscall.h"

void syscall_init(void)
{
    asm_write_msr(MSR_EFER, asm_read_msr(MSR_EFER) | 1);
    asm_write_msr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE * 8) << STAR_KCODE_OFFSET) | ((uint64_t)(((GDT_USER_DATA - 1) * 8) | 3) << STAR_UCODE_OFFSET));
    asm_write_msr(MSR_LSTAR, (uint64_t)syscall_handle);
    asm_write_msr(MSR_SYSCALL_FLAG_MASK, 0xfffffffe);
}

void syscall_set_gs(uintptr_t addr)
{
    asm_write_msr(MSR_GS_BASE, addr);
    asm_write_msr(MSR_KERN_GS_BASE, addr);
}

void syscall_handler(HalRegs *regs)
{
    regs->rax = _syscall_handler(regs->rax, (SysArgs){regs->rdi, regs->rsi, regs->rdx, regs->r8, regs->r9, regs->r10});
    return;
}
