#pragma once

#include <navy>
#include <result>
#include <stdint.h>

typedef enum
{
    IPC_PORT_SEND = 1 << 0,
    IPC_PORT_RECV = 1 << 1,
    IPC_PORT_SEND_ONCE = 1 << 2,
    IPC_PORT_RECV_ONCE = 1 << 3,
} IPCPortRights;

#ifdef __ck_sys_kernel__

typedef struct
{
    void *data;
    pid_t owner;
    pid_t peer;
    IPCPortRights rights;
} IpcPort;

typedef struct _IpcPortNode
{
    size_t id;
    IpcPort port;
    struct _IpcPortNode *next;
} IpcPortNode;

typedef struct
{
    IpcPortNode *head;
    IpcPortNode *tail;
} IpcPortList;

#    include <sched>

Res port_allocate_client(pid_t task_id, uint64_t rights);
Res port_allocate_both(pid_t client_id, pid_t server_id, uint64_t rights);

#else

typedef uintptr_t IpcPort;
Res port_alloc(uint64_t rights);

#endif // __ck_sys_kernel__
