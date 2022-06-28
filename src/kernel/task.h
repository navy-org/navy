#pragma once 

#include <copland/base.h>
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
    Range stack;
    TaskState state;
    intptr_t return_value;
    
    Context ctx;
} Task;

#define __task_create_impl2(n, ...) task_create ## n(__VA_ARGS__)
#define __task_create_impl(n, ...)  __task_create_impl2(n, __VA_ARGS__)
#define task_create(...) __task_create_impl(GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

Task *task_create1(Str path);
Task *task_create2(Str path, Pml *space);