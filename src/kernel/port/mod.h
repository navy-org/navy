#pragma once

#include <pmm>
#include <result>
#include <stdint.h>
#include <string.h>

typedef enum
{
    IPC_PORT_SEND = 1 << 0,
    IPC_PORT_RECV = 1 << 1,
    IPC_PORT_SEND_ONCE = 1 << 2,
} IPCPortRights;

typedef struct _PhysNode
{
    PhysObj obj;
    struct _PhysNode *next;
} PhysNode;

typedef struct
{
    PhysNode *head;
    PhysNode *tail;
} PhysList;

typedef struct
{
    PhysList objs;
    size_t owner;
    size_t peer;
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

Res port_allocate(size_t task_id, size_t peer, uint64_t rights);
Res port_allocate_both(size_t client_id, size_t server_id, uint64_t rights);
Res port_find(size_t task_id, uintptr_t port_id);
Res port_find_peer(IpcPort *port);
