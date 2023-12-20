#include <dbg/log.h>
#include <hal.h>

#include "asm.h"

void hal_regs_dump(HalRegs const *regs)
{
    uint64_t cr0, cr2, cr3, cr4;

    asm_read_cr(0, cr0);
    asm_read_cr(2, cr2);
    asm_read_cr(3, cr3);
    asm_read_cr(4, cr4);

    log$("=== Registers Dump ============================================");
    log$("interrupt: %x, err: %x", regs->intno, regs->err);
    log$("RAX %p RBX %p RCX %p RDX %p", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    log$("RSI %p RDI %p RBP %p RSP %p", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    log$("R8  %p R9  %p R10 %p R11 %p", regs->r8, regs->r9, regs->r10, regs->r11);
    log$("R12 %p R13 %p R14 %p R15 %p", regs->r12, regs->r13, regs->r14, regs->r15);
    log$("CR0 %p CR2 %p CR3 %p CR4 %p", cr0, cr2, cr3, cr4);
    log$("CS  %p SS  %p FLG %p", regs->cs, regs->ss, regs->rflags);
    log$("RIP \033[7m%p\033[0m", regs->rip);
    log$("==============================================================");
}