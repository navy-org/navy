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
    Task *task = (Task *)try$(sched_current());
    hal_space_unmap(task->space, (uintptr_t)ptr, align_up$(len, PMM_PAGE_SIZE));
    pmm_free((PhysObj){.base = (uintptr_t)ptr, .len = align_up$(len, PMM_PAGE_SIZE)});
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
    IpcPort *dst = (IpcPort *)try$(port_find_peer(src));

    if (src->rights & IPC_PORT_SEND_ONCE)
    {
        src->rights &= ~IPC_PORT_SEND_ONCE;
    }
    else if ((src->rights & IPC_PORT_SEND) == 0)
    {
        critical$("IPC security violation: %s tried to send to %s", task->name, ((Task *)try$(sched_get(dst->owner)))->name);
        return err$(RES_DENIED);
    }

    hal_space_unmap(task->space, (uintptr_t)data, align_up$(size, PMM_PAGE_SIZE));

    uintptr_t phys = try$(hal_virt2phys(task->space, (uintptr_t)data));
    PhysNode **node = dst->objs.head == NULL ? &dst->objs.head : &dst->objs.tail->next;

    *node = (PhysNode *)try$(kmalloc_acquire().calloc(1, sizeof(PhysNode)));
    (*node)->obj = (PhysObj){.base = phys, .len = size};
    dst->objs.tail = *node;

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
            if (node->port.objs.head != NULL)
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

    if ((self->rights & IPC_PORT_RECV) == 0)
    {
        critical$("IPC security violation: %s tried to receive from %s", task->name, ((Task *)try$(sched_get(self->peer)))->name);
        return err$(RES_DENIED);
    }

    while (self->objs.head == NULL)
        ;

    PhysObj *dataptr = &self->objs.head->obj;

    *data = vmem_alloc(&task->vmem, align_up$(dataptr->len, PMM_PAGE_SIZE), VM_INSTANTFIT);
    try$(hal_space_map(task->space, (uintptr_t)(*data), dataptr->base, align_up$(dataptr->len, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

    PhysNode *tmp = self->objs.head;
    self->objs.head = self->objs.head->next;
    kmalloc_acquire().free(tmp);

    return ok$();
}

static Res do_port_join(uintptr_t clientPort, uintptr_t serverPort, uintptr_t *newPort, uint64_t rights)
{
    Task *task = (Task *)try$(sched_current());
    if (task->pid != 1)
    {
        critical$("Security violation: %s tried to join ports", task->name);
        return err$(RES_DENIED);
    }

    IpcPort *client = (IpcPort *)try$(port_find(task->pid, clientPort));
    IpcPort *server = (IpcPort *)try$(port_find(task->pid, serverPort));

    try$(port_allocate_both(client->peer, server->peer, rights));

    Task *clientTask = (Task *)try$(sched_get(client->peer));
    *newPort = clientTask->ports.tail->id;

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
    [SYS_PORT_JOIN] = (Handler *)do_port_join,
};

Res _syscall_handler(Syscalls no, SysArgs args)
{
    if (no >= __SYSCALL_LENGTH)
    {
        return err$(RES_BADSYSCALL);
    }

    return handlers[no](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}
