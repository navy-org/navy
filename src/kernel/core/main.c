#include <elfloader>
#include <errno.h>
#include <hal>
#include <logger>
#include <pmm>
#include <sched>

_Noreturn int _start()
{
    hal_setup();

    long sched = sched_init();
    if (IS_ERR_VALUE(sched))
    {
        error$("Coudln't initialize scheduler (Err: %d)", (int)-sched);
        hal_panic();
    }

    for (;;)
        ;
}
