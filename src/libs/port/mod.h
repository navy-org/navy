#pragma once

#include <navy>
#include <result>
#include <stdint.h>
#include <string.h>

typedef enum
{
    IPC_PORT_SEND = 1 << 0,
    IPC_PORT_RECV = 1 << 1,
    IPC_PORT_SEND_ONCE = 1 << 2,
} IPCPortRights;

#ifdef __ck_sys_kernel__

#    include <pmm>

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

Res port_allocate(pid_t task_id, pid_t peer, uint64_t rights);
Res port_allocate_both(pid_t client_id, pid_t server_id, uint64_t rights);
Res port_find(pid_t task_id, uintptr_t port_id);
Res port_find_peer(IpcPort *port);

#else

typedef uintptr_t IpcPort;

#endif // __ck_sys_kernel__

#define send$(PORT, STRUCT) ({                         \
    void *buffer = NULL;                               \
    try$(sys_alloc(&buffer, sizeof(STRUCT)));          \
    memcpy(buffer, (void *)(&STRUCT), sizeof(STRUCT)); \
    sys_port_send(PORT, buffer, sizeof(STRUCT));       \
})

#define recv$(PORT, STRUCT) ({                               \
    __attribute__((__cleanup__(free_##STRUCT))) STRUCT *msg; \
    try$(sys_port_recv(PORT, (void **)&msg));                \
    msg;                                                     \
})

#define proto$(NAME)                           \
    NAME;                                      \
    static inline void free_##NAME(NAME **msg) \
    {                                          \
        sys_dealloc(msg, sizeof(NAME));        \
        *msg = NULL;                           \
    }
