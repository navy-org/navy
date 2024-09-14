#include <kmalloc>
#include <logging>
#include <sync>

#include "mod.h"

static Sched sched = {0};
static Alloc kmalloc;

Res sched_init(void)
{
    kmalloc = kmalloc_acquire();
    Task *kernel = (Task *)try$(task_new("kernel", uok$(hal_space_kernel()), err$(RES_NOENT)));

    SchedNode *node = (SchedNode *)try$(kmalloc.calloc(1, sizeof(SchedNode)));
    node->task = kernel;

    sched.current = node;
    sched.head = node;
    sched.tail = node;

    log$("Scheduler initialized");

    return ok$();
}

Res sched_add(Task *task)
{
    SchedNode *node = (SchedNode *)try$(kmalloc.calloc(1, sizeof(SchedNode)));
    node->task = task;
    node->next = sched.head;

    sched.tail->next = node;
    sched.tail = node;

    return ok$();
}

void sched_yield(HalRegs *regs)
{
    if (sched.current == NULL || (sched.current == sched.head && sched.current->next == NULL))
    {
        return;
    }

    hal_disable_interrupts();

    static size_t ticks = 0;

    if (++ticks > SCHED_QUANTUM)
    {
        ticks = 0;
        hal_context_save(sched.current->task->ctx, regs);
        sched.current = sched.current->next;
        hal_context_restore(sched.current->task->ctx, regs);
        hal_space_apply(sched.current->task->space);
    }
    hal_enable_interrupts();
}

Res sched_current(void)
{
    if (sched.current == NULL)
    {
        return err$(RES_NOENT);
    }

    return uok$(sched.current->task);
}

Res sched_get(pid_t pid)
{
    SchedNode *node = sched.head;

    while (node != sched.tail)
    {
        if (node->task->pid == pid)
        {
            return uok$((uintptr_t)node->task);
        }

        node = node->next;
    }

    return err$(RES_NOENT);
}