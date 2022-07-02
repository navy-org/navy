#include "cpu.h"

static Cpu *cpus;
static size_t count;

Cpu *cpu(int id)
{
    return &cpus[id];
}

CpuImpl *cpuimpl(int id)
{
    return cpus[id].impl;
}

CpuImpl *cpuimpl_self(void)
{
    return cpus[cpu_id()].impl;
}

Cpu *cpu_self(void)
{
    return &cpus[cpu_id()];
}

void cpu_preinit(Handover const *handover)
{
    count = handover->smp.core_count;
    cpus = calloc(sizeof(CpuImpl), count);

    for (size_t i = 0; i < count; i++)
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

size_t cpu_count(void)
{
    return count;
}