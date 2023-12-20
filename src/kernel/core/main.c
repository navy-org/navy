#include <dbg/log.h>
#include <hal.h>
#include <tinyvmem/tinyvmem.h>

#include "pmm.h"
#include "pre-sched.h"

_Noreturn int _start()
{
    Res pmm = pmm_init();
    if (pmm.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize PMM",
               res_to_str(pmm), pmm.loc.func, pmm.loc.file, pmm.loc.line);
        hal_panic();
    }

    Res hal = hal_setup();
    if (hal.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize HAL",
               res_to_str(hal), hal.loc.func, hal.loc.file, hal.loc.line);
        hal_panic();
    }

    Res sched = load_scheduler();
    if (sched.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize scheduler",
               res_to_str(sched), sched.loc.func, sched.loc.file, sched.loc.line);
        hal_panic();
    }

    for (;;)
        ;
}