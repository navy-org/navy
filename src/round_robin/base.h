#pragma once 

#define SCHED_QUATUM 8

#include <kernel/task.h>

void sched_init(void);
void sched_yield(Regs *regs);
void sched_push_task(Task *task);