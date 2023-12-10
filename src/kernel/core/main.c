#include <dbg/log.h>
#include <hal.h>

_Noreturn int _start()
{
    log$("Hello, world!");
    hal_setup();

    for (;;)
        ;
}