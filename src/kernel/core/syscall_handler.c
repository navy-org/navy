#include <hal>
#include <kmalloc>
#include <logging>
#include <navy>
#include <pmm>
#include <result>
#include <sched>
#include <tinyvmem>

typedef Res Handler(SysArg arg1, SysArg arg2, SysArg arg3, SysArg arg4, SysArg arg5, SysArg arg6);

static Res do_log(char const *s, size_t len)
{
    return hal_dbg_stream().write(len, s);
}

static Res do_alloc(void **ptr, size_t size)
{
    PmmObj page = pmm_alloc(align_up$(size, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
    if (page.len == 0)
    {
        return err$(RES_NOMEM);
    }

    Task *task = (Task *)try$(sched_current());
    uintptr_t vaddr = (uintptr_t)vmem_alloc(&task->vmem, size, VM_INSTANTFIT);
    *ptr = (void *)vaddr;
    try$(hal_space_map(task->space, vaddr, page.base, size, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

    return ok$();
}

static Res do_dealloc(void *ptr, size_t len)
{
    pmm_free((PmmObj){.base = (uintptr_t)ptr, .len = len});
    return ok$();
}

static Res do_port_alloc(uintptr_t *port, uint64_t rights)
{
    Task *task = (Task *)try$(sched_current());
    port_allocate_client(task->pid, rights);
    *port = task->ports.tail->id;
    return ok$();
}

static Res do_port_deinit(uintptr_t *port)
{
    Task *task = (Task *)try$(sched_current());
    IpcPortList *lst = &task->ports;
    IpcPortNode *node = lst->head;
    IpcPortNode *tmp;

    while (node != NULL)
    {
        if (node->id == *port)
        {
            tmp = node;
            node = node->next;
            try$(kmalloc_acquire().free(tmp));
        }

        node = node->next;
    }

    return ok$();
}

static Handler *handlers[__SYSCALL_LENGTH] = {
    [SYS_LOG] = (Handler *)do_log,
    [SYS_ALLOC] = (Handler *)do_alloc,
    [SYS_DEALLOC] = (Handler *)do_dealloc,
    [SYS_PORT_ALLOC] = (Handler *)do_port_alloc,
    [SYS_PORT_DEINIT] = (Handler *)do_port_deinit,
};

Res _syscall_handler(Syscalls no, SysArgs args)
{
    if (no >= __SYSCALL_LENGTH)
    {
        return err$(RES_BADSYSCALL);
    }

    return handlers[no](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}