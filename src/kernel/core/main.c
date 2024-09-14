#include <elfloader>
#include <hal>
#include <logging>
#include <pmm>
#include <sched>
#include <tinyvmem>

_Noreturn int _start()
{
    vmem_bootstrap();

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

    Res bootstrap = elfloader_instantiate("/bin/bootstrap");
    if (bootstrap.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't instantiate /bin/bootstrap",
               res_to_str(bootstrap), bootstrap.loc.func, bootstrap.loc.file, bootstrap.loc.line);
        hal_panic();
    }

    Res hello = elfloader_instantiate("/bin/hello");
    if (hello.type != RES_OK)
    {
        error$("%s at %s (%s:%d) - Couldn't instantiate /bin/hello",
               res_to_str(hello), hello.loc.func, hello.loc.file, hello.loc.line);
        hal_panic();
    }

    for (;;)
        ;
}