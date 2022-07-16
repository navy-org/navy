#include "base.h"
#include "x86_64/ctx.h"
#include "x86_64/vmm.h"

#include <kernel/abstraction.h>
#include <kernel/spinlock.h>
#include <kernel/cpu.h>

static uint32_t lock;

void sched_init(void)
{
    Task *boot = task_create(str$("Boot"), vmm_get_kernel_space());
    for (size_t i = 0; i < cpu_count(); i++)
    {
        cpu(i)->current = 0;
        cpu(i)->tick = 0;
        vec_push(&cpu(i)->tasks, boot);
    }
}

void sched_yield(Regs *regs)
{
    lock$(lock);

    if (cpu_self()->tick++ > SCHED_QUATUM && cpu_self()->tasks.length > 1)
    {
        cpu_self()->tick = 0;
        Task *current_task = cpu_self()->tasks.data[cpu_self()->current];
        context_save(&current_task->ctx, regs);
        log$("Current task was {} (ip = {a})", current_task->path, current_task->ctx.regs.rip);

        loop 
        {
            cpu_self()->current++;

            if (cpu_self()->tasks.length == cpu_self()->current)
            {
                cpu_self()->current = 0;
            }

            current_task = cpu_self()->tasks.data[cpu_self()->current];

            if (current_task->state == TASK_READY)
            {
                break;
            }
        }

        if (cpu_self()->id == 0)
        {
            log$("Switching to {} (ip = {a})", current_task->path, current_task->ctx.regs.rip);            
        }

        context_switch(&current_task->ctx, regs);
        vmm_switch_space(current_task->space);
    }

    unlock$(lock);
}


void sched_push_task(Task *task)
{
    size_t smallest = cpu(0)->tasks.length;
    int cpu_id = 0;

    for (size_t i = 1; i < cpu_count(); i++)
    {
        if (smallest < cpu(i)->tasks.length)
        {
            smallest = i;
            cpu_id = i;
        }
    }

    vec_push(&cpu(cpu_id)->tasks, task);
}