#pragma once 

#include "base.h"

typedef struct 
{
    Tss tss;
} CpuImpl;

void cpuimpl_init(CpuImpl *self);
void cpu_init(void);
int cpu_id(void);