#include <logging>
#include <port>
#include <result>
#include <string.h>

Res main(int argc, char const **argv)
{
    [[gnu::unused]] IpcPort *port = (IpcPort *)try$(port_alloc(IPC_PORT_RECV));

    log$("Argc = %d", argc);
    log$("Hello, World from %s !", argv[0]);
    return ok$();
}