#include <errno.h>
#include <kmalloc>
#include <logger>
#include <sync>

#include "mod.h"

static Sched sched = {0};

long sched_init(void)
{
    Task *kernel = task_new("kernel", hal_space_kernel(), 0);
    if (IS_ERR(kernel))
    {
        return PTR_ERR(kernel);
    }

    SchedNode *node = (SchedNode *)kmalloc_calloc(1, sizeof(SchedNode));
    if (IS_ERR(node))
    {
        return PTR_ERR(node);
    }
    node->task = kernel;

    sched.current = node;
    sched.head = node;
    sched.tail = node;

    log$("Scheduler initialized");

    return 0;
}

long sched_add(Task *task)
{
    SchedNode *node = (SchedNode *)kmalloc_calloc(1, sizeof(SchedNode));
    if (IS_ERR(node))
    {
        return PTR_ERR(node);
    }

    node->task = task;
    node->next = NULL;

    sched.tail->next = node;
    sched.tail = node;

    return 0;
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

        if (sched.current == NULL)
        {
            sched.current = sched.head;
        }

        hal_context_restore(sched.current->task->ctx, regs);
        hal_space_apply(sched.current->task->space);
    }

    hal_enable_interrupts();
}

Task *sched_current(void)
{
    if (sched.current == NULL)
    {
        return ERR_PTR(-ENOENT);
    }

    return sched.current->task;
}

Task *sched_get(size_t pid)
{
    SchedNode *node = sched.head;

    while (node != NULL)
    {
        if (node->task->pid == pid)
        {
            return node->task;
        }

        node = node->next;
    }

    return ERR_PTR(-ENOENT);
}
