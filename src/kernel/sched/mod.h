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

long sched_init(void);

long sched_add(Task *task);

void sched_yield(HalRegs *regs);

Task *sched_current(void);

Task *sched_get(size_t pid);
