#include <hal>
#include <result>

Res _syscall_handler([[gnu::unused]] Syscalls no, [[gnu::unused]] SysArgs args)
{
    return ok$();
}
