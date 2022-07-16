#pragma once 

#include <copland/base.h>
#include <unistd.h>
#include "abstraction.h"

typedef enum 
{
    TASK_DORMANT,
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_ISR
} TaskState;

typedef struct 
{
    Str path;
    Pml *space;
    pid_t pid;
    Range stack;
    TaskState state;
    intptr_t return_value;
    
    Context ctx;
} Task;

typedef Vec(Task *) TaskQueue;
Task *task_create_impl(Str path, Pml *space);

#define __task_create(path, pml, ...) task_create_impl(path, pml)
#define task_create(...) __task_create(__VA_ARGS__, 0)