#include <hal>
#include <logging>
#include <pmm>
#include <tinyvmem>

_Noreturn int _start()
{
    Res pmm = pmm_init();
    if (pmm.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize PMM",
               res_to_str(pmm), pmm.loc.func, pmm.loc.file, pmm.loc.line);
        hal_panic();
    }

    vmem_bootstrap();

    Res hal = hal_setup();
    if (hal.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't initialize HAL",
               res_to_str(hal), hal.loc.func, hal.loc.file, hal.loc.line);
        hal_panic();
    }

    for (;;)
        ;
}