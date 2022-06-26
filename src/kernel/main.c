#include "abstraction.h"
#include "pmm.h"
#include "const.h"

#include <copland/base.h>
#include <handover/handover.h>

#include <limine/base.h>
#include <limine/requests.h>

int _start(void)
{
    Com debug = com_init(COM1);
    define_debug_out((Writer *) &debug);

    ResultHandover resHandover = handover_create();
    if (!resHandover.succ)
    {
        panic$("{}", resHandover.err);
    }

    Handover handover = UNWRAP(resHandover);
    set_hhdm_offset(handover.hhdm_offset);
    pmm_init(&handover);
    hardware_init(&handover);

    loop;
}