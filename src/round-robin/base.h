#pragma once 

#define SCHED_QUATUM 8

#include <kernel/abstraction.h>

void sched_init(void);
void sched_yield(Regs *regs);