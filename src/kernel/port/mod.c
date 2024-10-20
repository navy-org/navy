#include <kmalloc>
#include <port>
#include <result>
#include <sched>

static uint64_t invert_rights(uint64_t rights)
{
    uint64_t inv = 0;

    if (rights & IPC_PORT_RECV)
    {
        inv |= IPC_PORT_SEND;
    }

    if (rights & IPC_PORT_SEND)
    {
        inv |= IPC_PORT_RECV;
    }

    if (rights & IPC_PORT_SEND_ONCE)
    {
        inv |= IPC_PORT_RECV;
    }

    return inv;
}

Res port_allocate(size_t task_id, size_t peer, uint64_t rights)
{
    Task *task = (Task *)try$(sched_get(task_id));
    IpcPortList *lst = &task->ports;
    IpcPortNode **node = NULL;
    uint64_t port_id;

    if (lst->head == NULL)
    {
        port_id = 0;
        node = &lst->head;
    }
    else
    {
        port_id = lst->tail->id + 1;
        node = &lst->tail->next;
    }

    if (*node != NULL)
    {
        return err$(RES_INVAL);
    }

    *node = (IpcPortNode *)try$(kmalloc_acquire().calloc(1, sizeof(IpcPortNode)));
    (*node)->id = port_id;
    (*node)->port = (IpcPort){
        .objs = {0},
        .rights = rights,
        .peer = peer,
        .owner = task_id,
    };

    lst->tail = *node;

    return ok$();
}

Res port_allocate_both(size_t client_id, size_t server_id, uint64_t rights)
{
    try$(port_allocate(client_id, server_id, rights));
    try$(port_allocate(server_id, client_id, invert_rights(rights)));
    return ok$();
}

Res port_find(size_t task_id, uintptr_t port_id)
{
    Task *task = (Task *)try$(sched_get(task_id));

    IpcPortList *lst = &task->ports;
    IpcPortNode *node = lst->head;

    while (node != NULL)
    {
        if (node->id == port_id)
        {
            return uok$(&node->port);
        }

        node = node->next;
    }

    return err$(RES_BADPORT);
}

Res port_find_peer(IpcPort *port)
{
    Task *task = (Task *)try$(sched_get(port->peer));
    IpcPortList *lst = &task->ports;
    IpcPortNode *node = lst->head;

    while (node != NULL)
    {
        if (node->port.peer == port->owner)
        {
            return uok$(&node->port);
        }

        node = node->next;
    }

    return err$(RES_BADPORT);
}
