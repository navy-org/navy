#include <stdint.h>
#include <stdlib.h>
#include <copland/base.h>

#include "base.h"

static char *_exception_messages[32] = {
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
    "VMM Communication Exception",
    "Security Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};


static const char *comments[] = {
    "Panic At The Kernel !",
    "Press F to pay respect.",
    "Could not boot up kernel because no",
    "Booting up is not supported on this kernel",
    "Seems like Duolingo halted the kernel",
    "Hey /r/softwaregore",
    "Believe me, everything is fine :)",
    "The kernel defined that 1 + 1 = 3",
    "Congratulations, this kernel can now run Half Life 3 !",
    "Still more stable than Windows",
    "Looks like the CPU took a lil' nap",
    "Stability is bloat",
    "The kernel took some vacations",
    "https://www.youtube.com/watch?v=dQw4w9WgXcQ",
    "Still faster developpement than GNU Hurd",
    "Did you try to rewrite the kernel in rust ?",
    "Better of doing a megalithicc kernel",
    "The dev of this kernel is the worst at copying and pasting",
    "Maybe if you create your own cpu architecture it will work",
    "DevseChan, what if we kissed during the kernel panic",
    "\033[5mPaging hell is real\033[0m",
    "Calculating infinity like it's 1999 !",
    "The daughter of the seastar and Chopin is a fraud believe me.",
    "This kernel listened too much to k-pop, that's why !",
    "Keyboard not found! Please press any key to continue...",
    "Time to put on the black stripped programming socks",
    "Time to put on the blue stripped programming socks",
    "Time to put on the pink stripped programming socks",
    "Time to put on the programming hoody",
    "Time to put on the programming skirt",
    "You've been terminated"
};

static void exception_handler(Regs *regs)
{
    Writer *debug = get_debug_out();

    srandom(rtc_sec() * rtc_sec());
    int funny_id = random() % (sizeof(comments) / sizeof(comments[0]));

    fmt$(debug, "\033[31m!!\033[33m-----------------------------------------------------------------------------------\033[0m\n");
    fmt$(debug, "\n\tKERNEL PANIC\n\t\033[51m{}\033[0m\n\t", comments[funny_id]);
    fmt$(debug, "{} (0x{x}) Err: {d}\n\n", _exception_messages[regs->intno], regs->intno, regs->err);
    fmt$(debug, "  RAX {a} RBX {a} RCX {a} RDX {a}\n", regs->rax,
                 regs->rbx, regs->rcx, regs->rdx);
    fmt$(debug, "  RSI {a} RDI {a} RBP {a} RSP {a}\n", regs->rsi,
                 regs->rdi, regs->rbp, regs->rsp);
    fmt$(debug, "   R8 {a}  R9 {a} R10 {a} R11 {a}\n", regs->r8,
                 regs->r9, regs->r10, regs->r11);
    fmt$(debug, "  R12 {a} R13 {a} R14 {a} R15 {a}\n", regs->r12,
                 regs->r13, regs->r14, regs->r15);
    fmt$(debug, "  CR0 {a} CR2 {a} CR3 {a} CR4 {a}\n", read_cr0(), read_cr2(), read_cr3(), read_cr4());
    fmt$(debug, "  RIP \033[7m{a}\033[0m\n", regs->rip);
    fmt$(debug, "\n\033[33m-----------------------------------------------------------------------------------\033[31m!!\033[0m\n");
}

uint64_t interrupts_handler(uint64_t rsp)
{
    Regs *regs = (Regs *) rsp;

    if (regs->intno < 32)
    {
        exception_handler(regs);
        
        loop
        {
            cli();
            hlt();
        }
    }

    else if (regs->intno >= 48)
    {
        log$("{}", regs->intno);
    }

    lapic_eoi();
    return rsp;
}