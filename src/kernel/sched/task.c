#include <ds>
#include <errno.h>
#include <hal>
#include <kmalloc>
#include <logger>
#include <pmm>
#include <sched>
#include <string.h>
#include <sync>

#include "task.h"

Task *task_new(char const *name, HalPage *address_space, uintptr_t ip)
{
    static size_t pid = 0;

    Alloc kmalloc = kmalloc_acquire();
    Task *task = (Task *)kmalloc.calloc(1, sizeof(Task));
    if (IS_ERR(task))
    {
        return task;
    }

    task->name = (char *)kmalloc.calloc(1, strlen(name) + 1);
    if (IS_ERR(task->name))
    {
        task_destroy(task);
        return ERR_CAST(task->name);
    }

    task->pid = pid++;
    memcpy((void *)task->name, name, strlen(name));

    if (address_space == NULL)
    {
        task->space = address_space;
    }
    else
    {
        hal_space_create(&task->space);
    }

    task->ctx = hal_context_create();
    if (IS_ERR(task->ctx))
    {
        task_destroy(task);
        return ERR_CAST(task->ctx);
    }

    if (task->pid > 0)
    {
        PhysObj stack = pmm_alloc(USER_STACK_SIZE / PMM_PAGE_SIZE);
        if (stack.len == 0)
        {
            task_destroy(task);
            return ERR_PTR(-ENOMEM);
        }

        long err = hal_space_map(task->space, USER_STACK_BASE, stack.base, USER_STACK_SIZE, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER);
        if (IS_ERR_VALUE(err))
        {
            pmm_free(stack);
            task_destroy(task);
            return ERR_PTR(err);
        }

        Stack stack_obj = stack_init(hal_mmap_l2h(stack.base), USER_STACK_SIZE);

        char *argv[1] = {(char *)USER_STACK_TOP - stack_push(&stack_obj, (void *)name, strlen(name) + 1)};
        uintptr_t argc = 1;

        stack_push(&stack_obj, &argv, sizeof(char *));
        stack_push(&stack_obj, (void *)&argc, sizeof(uintptr_t));

        if (ip > 0)
        {
            hal_context_start(task->ctx, ip, USER_STACK_TOP - stack_obj.off);
        }
        else
        {
            hal_context_start(task->ctx, 0, USER_STACK_TOP - stack_obj.off);
        }

        err = sched_add(task);
        if (IS_ERR_VALUE(err))
        {
            pmm_free(stack);
            task_destroy(task);
            return ERR_PTR(err);
        }
    }
    else
    {
        long err = hal_context_start(task->ctx, ip, 0);
        if (IS_ERR_VALUE(err))
        {
            task_destroy(task);
            return ERR_PTR(err);
        }
    }

    return task;
}

void task_destroy(Task *task)
{
    Alloc kmalloc = kmalloc_acquire();

    if (!IS_ERR_OR_NULL(task->name))
    {
        kmalloc.free((void *)task->name);
    }

    if (!IS_ERR_OR_NULL(task))
    {
        kmalloc.free((void *)task);
    }
}
