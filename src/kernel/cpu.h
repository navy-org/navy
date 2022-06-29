#pragma once

#include "task.h"

#include <handover/handover.h>

typedef struct 
{
    int id;
    TaskQueue tasks;
    CoreGoto func;
} Cpu;

void cpu_preinit(Handover const *handover);
void cpu_idle(void);

CpuImpl *cpuimpl(int id);
CpuImpl *cpuimpl_self(void);

Cpu *cpu(int id);
Cpu *cpu_self(void);