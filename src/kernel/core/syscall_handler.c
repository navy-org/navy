#include <dbg/log.h>
#include <navy/api.h>
#include <res.h>

#include "hal.h"

typedef Res Handler(SysArg arg1, SysArg arg2, SysArg arg3, SysArg arg4, SysArg arg5, SysArg arg6);

static Res _sys_log(char const *s, size_t len)
{
    return hal_dbg_stream().write(len, s);
}

static Handler *handlers[__SYSCALL_LENGTH] = {
    [SYS_LOG] = (Handler *)_sys_log,
};

Res _syscall_handler(Syscalls no, SysArgs args)
{
    if (no >= __SYSCALL_LENGTH)
    {
        return err$(RES_BADSYSCALL);
    }

    return handlers[no](args.arg1, args.arg2, args.arg3, args.arg4, args.arg5, args.arg6);
}