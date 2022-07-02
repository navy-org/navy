#pragma once

#include "task.h"
#include <handover/handover.h>

typedef struct 
{
    int id;
    uint8_t tick;
    TaskQueue tasks;
    size_t current;
    CoreGoto func;
    CpuImpl *impl;
} Cpu;

void cpu_preinit(Handover const *handover);
void cpu_idle(void);

CpuImpl *cpuimpl(int id);
CpuImpl *cpuimpl_self(void);

Cpu *cpu(int id);
Cpu *cpu_self(void);

size_t cpu_count(void);