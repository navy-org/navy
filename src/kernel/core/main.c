#include <dbg/log.h>

_Noreturn int _start()
{
    log$("Hello, world!");
    for (;;)
        ;
}