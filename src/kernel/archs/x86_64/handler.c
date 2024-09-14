#include <hal>
#include <logging>
#include <sched>
#include <sync>

#include "apic.h"
#include "asm.h"
#include "regs.h"

enum
{
    IRQ0 = 32,
};

static Spinlock lock = SPINLOCK_INIT;
static char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Detected Overflow",
    "Out Of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad Tss",
    "Segment Not Present",
    "StackFault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "paging Communication Exception",
    "Security Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

static void dump_backtrace(uintptr_t rbp)
{
    struct _StackFrame
    {
        struct _StackFrame *rbp;
        uint64_t rip;
    } *stackframe = (void *)rbp;

    for (;;)
    {
        if (stackframe->rip == 0)
        {
            break;
        }
        print$("     * %p\n", stackframe->rip);
        stackframe = stackframe->rbp;
    }
}

static void kpanic(HalRegs const regs[static 1])
{
    spinlock_acquire(&lock);

    uint64_t cr0, cr2, cr3, cr4;
    Res task = sched_current();

    asm_read_cr(0, cr0);
    asm_read_cr(2, cr2);
    asm_read_cr(3, cr3);
    asm_read_cr(4, cr4);

    print$("\n!!! ---------------------------------------------------------------------------------------------------\n\n");
    print$("    KERNEL PANIC\n");
    print$("    %s was raised\n\n", exception_messages[regs->intno]);

    if (task.type == RES_OK)
    {
        print$("    task: %s\n", ((Task *)task.uvalue)->name);
    }
    else
    {
        print$("    task: kernel\n");
    }

    print$("    interrupt: %x, err: %x \n", regs->intno, regs->err);
    print$("    RAX %p RBX %p RCX %p RDX %p\n", regs->rax, regs->rbx, regs->rcx, regs->rdx);
    print$("    RSI %p RDI %p RBP %p RSP %p\n", regs->rsi, regs->rdi, regs->rbp, regs->rsp);
    print$("    R8  %p R9  %p R10 %p R11 %p\n", regs->r8, regs->r9, regs->r10, regs->r11);
    print$("    R12 %p R13 %p R14 %p R15 %p\n", regs->r12, regs->r13, regs->r14, regs->r15);
    print$("    CR0 %p CR2 %p CR3 %p CR4 %p\n", cr0, cr2, cr3, cr4);
    print$("    CS  %p SS  %p FLG %p\n", regs->cs, regs->ss, regs->rflags);
    print$("    RIP \033[7m%p\033[0m\n\n", regs->rip);
    print$("    Backtrace:\n");
    dump_backtrace(regs->rbp);
    print$("--------------------------------------------------------------------------------------------------- !!!\n\n");
    spinlock_release(&lock);
}

uintptr_t interrupt_handler(uintptr_t rsp)
{
    HalRegs *regs = (HalRegs *)rsp;

    if (regs->intno < IRQ0)
    {
        kpanic(regs);

        for (;;)
        {
            hal_disable_interrupts();
            hal_pause();
        }
    }
    else if (regs->intno == IRQ0)
    {
        sched_yield(regs);
    }

    lapic_eoi();
    return rsp;
}