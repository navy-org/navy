#include "cpu.h"
#include "abstraction.h"
#include "handover/handover.h"

static CpuImpl *cpus;
static size_t cpu_count;

Cpu *cpu(int id)
{
    return (Cpu *) &cpus[id].base;
}

CpuImpl *cpuimpl(int id)
{
    return &cpus[id];
}

CpuImpl *cpuimpl_self(void)
{
    return &cpus[cpu_id()];
}

Cpu *cpu_self(void)
{
    return (Cpu *) &cpus[cpu_id()].base;
}

void cpu_preinit(Handover const *handover)
{
    cpu_count = handover->smp.core_count;
    cpus = calloc(sizeof(CpuImpl), cpu_count);

    for (size_t i = 0; i < cpu_count; i++)
    {
        vec_init(&cpu(i)->tasks);
        cpuimpl_init(cpuimpl(i));
        handover_goto_core(i, cpu_init);
    }
}

void cpu_idle(void)
{
    loop 
    {
        if (cpu_self()->func != NULL)
        {
            cpu_self()->func();
            cpu_self()->func = NULL;
        }
    }
}