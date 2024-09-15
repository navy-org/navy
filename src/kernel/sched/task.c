#include <ds>
#include <hal>
#include <kmalloc>
#include <logging>
#include <pmm>
#include <sched>
#include <string.h>
#include <sync>

#include "task.h"

Res task_new(char const *name, Res address_space, Res ip)
{
    static pid_t pid = 0;

    Alloc kmalloc = kmalloc_acquire();
    Task *task = (Task *)try$(kmalloc.calloc(1, sizeof(Task)));
    task->ports = (IpcPortList){0};

    task->name = (char *)try$(kmalloc.calloc(1, strlen(name) + 1));
    task->pid = pid++;
    memcpy((void *)task->name, name, strlen(name));

    if (address_space.type == RES_OK)
    {
        task->space = (HalPage *)address_space.uvalue;
    }
    else
    {
        hal_space_create(&task->space);
    }

    task->ctx = (HalContext *)try$(hal_context_create());

    if (task->pid > 0)
    {
        PhysObj stack = pmm_alloc(USER_STACK_SIZE / PMM_PAGE_SIZE);
        if (stack.len == 0)
        {
            return err$(RES_NOMEM);
        }

        try$(hal_space_map(task->space, USER_STACK_BASE, stack.base, USER_STACK_SIZE, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

        Stack stack_obj = stack_init(hal_mmap_l2h(stack.base), USER_STACK_SIZE);

        char *argv[1] = {(char *)USER_STACK_TOP - stack_push(&stack_obj, (void *)name, strlen(name) + 1)};
        uintptr_t argc = 1;

        stack_push(&stack_obj, &argv, sizeof(char *));
        stack_push(&stack_obj, (void *)&argc, sizeof(uintptr_t));

        if (ip.type == RES_OK)
        {
            hal_context_start(task->ctx, ip.uvalue, USER_STACK_TOP - stack_obj.off);
        }
        else
        {
            hal_context_start(task->ctx, 0, USER_STACK_TOP - stack_obj.off);
        }

        vmem_init(&task->vmem, (char *)name, (void *)USER_HEAP_BASE + PMM_PAGE_SIZE, USER_HEAP_SIZE, PMM_PAGE_SIZE, NULL, NULL, NULL, 0, 0);
        try$(sched_add(task));

        if (task->pid > 1)
        {
            try$(port_allocate_both(task->pid, 1, IPC_PORT_RECV_ONCE | IPC_PORT_SEND_ONCE));
        }
    }
    else
    {
        hal_context_start(task->ctx, ip.uvalue, 0);
    }

    return uok$(task);
}