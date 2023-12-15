#include <dbg/log.h>
#include <hal.h>

#include "pmm.h"

_Noreturn int _start()
{
    pmm_init();
    hal_setup();

    for (;;)
        ;
}