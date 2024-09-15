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
    size_t sz = align_up$(size, PMM_PAGE_SIZE);
    PhysObj page = pmm_alloc(sz / PMM_PAGE_SIZE);
    if (page.len == 0)
    {
        return err$(RES_NOMEM);
    }

    Task *task = (Task *)try$(sched_current());
    uintptr_t vaddr = (uintptr_t)vmem_alloc(&task->vmem, sz, VM_INSTANTFIT);
    *ptr = (void *)vaddr;
    try$(hal_space_map(task->space, vaddr, page.base, sz, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

    return ok$();
}

static Res do_dealloc(void *ptr, size_t len)
{
    pmm_free((PhysObj){.base = (uintptr_t)ptr, .len = len});
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

static Res do_port_send(uintptr_t port, void *data, size_t size)
{
    Task *task = (Task *)try$(sched_current());
    IpcPort *src = (IpcPort *)try$(port_find(task->pid, port));
    IpcPort *dst = (IpcPort *)try$(port_find_peer_port(src));

    if (src->rights & IPC_PORT_SEND_ONCE)
    {
        src->rights &= ~IPC_PORT_SEND_ONCE;
    }
    else if ((src->rights & IPC_PORT_SEND) == 0)
    {
        return err$(RES_DENIED);
    }

    uintptr_t phys = try$(hal_virt2phys(task->space, (uintptr_t)data));
    PhysObj *dataptr = &dst->dataptr[0];

    while (dataptr->len > 0)
    {
        dataptr++;
    }

    dataptr->base = phys;
    dataptr->len = size;

    return ok$();
}

static Res do_port_wild(uintptr_t *port)
{
    Task *task = (Task *)try$(sched_current());
    IpcPortNode *node;
    bool found = false;

    while (!found)
    {
        node = task->ports.head;

        while (node != NULL)
        {
            if (node->port.dataptr[0].len != 0)
            {
                *port = node->id;
                found = true;
            }

            node = node->next;
        }
    }

    return ok$();
}

static Res do_port_recv(uintptr_t port, void **data)
{
    Task *task = (Task *)try$(sched_current());
    IpcPort *self = (IpcPort *)try$(port_find(task->pid, port));

    if (self->rights & IPC_PORT_RECV_ONCE)
    {
        self->rights &= ~IPC_PORT_RECV_ONCE;
    }
    else if ((self->rights & IPC_PORT_RECV) == 0)
    {
        return err$(RES_DENIED);
    }

    PhysObj *dataptr = &self->dataptr[0];

    while (dataptr->len == 0)
        ;

    *data = vmem_alloc(&task->vmem, align_up$(dataptr->len, PMM_PAGE_SIZE), VM_INSTANTFIT);
    try$(hal_space_map(task->space, (uintptr_t)(*data), dataptr->base, align_up$(dataptr->len, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

    return ok$();
}

static Handler *handlers[__SYSCALL_LENGTH] = {
    [SYS_LOG] = (Handler *)do_log,
    [SYS_ALLOC] = (Handler *)do_alloc,
    [SYS_DEALLOC] = (Handler *)do_dealloc,
    [SYS_PORT_DEINIT] = (Handler *)do_port_deinit,
    [SYS_PORT_SEND] = (Handler *)do_port_send,
    [SYS_PORT_WILD] = (Handler *)do_port_wild,
    [SYS_PORT_RECV] = (Handler *)do_port_recv,
};

Res _syscall_handler(Syscalls no, SysArgs args)
{
    if (no >= __SYSCALL_LENGTH)
    {
        return err$(RES_BADSYSCALL);
    }

    return handlers[no](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}