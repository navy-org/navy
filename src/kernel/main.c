#include "abstraction.h"
#include "elf.h"
#include "pmm.h"

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

    sched_init();

    Module bin = handover_find_module(&handover, str$("/bin/hello-world"));
    (void) elf_load(&bin, (TaskArgs){});

    loop;
}