#include <elfloader>
#include <hal>
#include <logger>
#include <pmm>
#include <sched>

_Noreturn int _start()
{
    Res hal = hal_setup();
    if (hal.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize HAL",
               res_to_str(hal), hal.loc.func, hal.loc.file, hal.loc.line);
        hal_panic();
    }

    Res sched = sched_init();
    if (sched.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize scheduler",
               res_to_str(sched), sched.loc.func, sched.loc.file, sched.loc.line);
        hal_panic();
    }

    for (;;)
        ;
}
