#include <kmalloc>
#include <navy>
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
        inv |= IPC_PORT_RECV_ONCE;
    }

    if (rights & IPC_PORT_RECV_ONCE)
    {
        inv |= IPC_PORT_SEND_ONCE;
    }

    return inv;
}

Res port_allocate_client(pid_t task_id, uint64_t rights)
{
    Task *task = (Task *)try$(sched_get(task_id));
    IpcPortList *lst = &task->ports;
    IpcPortNode *node = NULL;
    uint64_t port_id;

    if (lst->head == NULL)
    {
        port_id = 0;
        node = lst->head;
    }
    else
    {
        port_id = lst->tail->id + 1;
        node = lst->tail->next;
    }

    if (node != NULL)
    {
        return err$(RES_INVAL);
    }

    node = (IpcPortNode *)try$(kmalloc_acquire().calloc(1, sizeof(IpcPortNode)));
    node->id = port_id;

    node->port = (IpcPort){
        .data = NULL,
        .rights = rights,
    };

    lst->tail = node;

    return ok$();
}

Res port_allocate_both(pid_t client_id, pid_t server_id, uint64_t rights)
{
    try$(port_allocate_client(client_id, rights));
    try$(port_allocate_client(server_id, invert_rights(rights)));
    return ok$();
}