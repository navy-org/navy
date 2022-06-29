#include "cpu.h"
#include "copland/debug.h"
#include "x86_64/madt.h"
#include <kernel/cpu.h>

#include <stdlib.h>

int cpu_id(void)
{
    return lapic_current_cpu();
}

void cpuimpl_init(CpuImpl *self)
{
    self->tss.ist[0] = (uint64_t) malloc(STACK_SIZE) + STACK_SIZE;
    self->tss.ist[1] = (uint64_t) malloc(STACK_SIZE) + STACK_SIZE;
    self->tss.rsp[0] = (uint64_t) malloc(STACK_SIZE) + STACK_SIZE;
}

void cpu_init(void)
{
    if (cpu_id() > 0)
    {
        gdt_init();
        idt_init();
    }

    syscall_init();
    gdt_load_tss(&cpuimpl_self()->tss);

    if (cpu_id() > 0) 
    {
        cpu_idle();
    }
}