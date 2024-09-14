#pragma once

#include "task.h"

#define SCHED_QUANTUM (64)

typedef struct _SchedNode
{
    Task *task;
    struct _SchedNode *next;
} SchedNode;

typedef struct
{
    SchedNode *head;
    SchedNode *tail;
    SchedNode *current;
} Sched;

Res sched_init(void);

Res sched_add(Task *task);

void sched_yield(HalRegs *regs);

Res sched_current(void);

Res sched_get(pid_t pid);