#pragma once 

#include "base.h"

typedef struct 
{
    void *base;
    Tss tss;
} CpuImpl;

void cpuimpl_init(CpuImpl *self);
void cpu_init(void);
int cpu_id(void);