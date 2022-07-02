#include "base.h"
#include "x86_64/ctx.h"
#include "x86_64/vmm.h"

#include <kernel/abstraction.h>
#include <kernel/task.h>
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

    if (++cpu_self()->tick > SCHED_QUATUM)
    {
        cpu_self()->tick = 0;
        Task *current_task = cpu_self()->tasks.data[cpu_self()->current];
        context_save(&current_task->ctx, regs);

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

        context_switch(&current_task->ctx, regs);
        vmm_switch_space(current_task->space);
    }

    unlock$(lock);
}