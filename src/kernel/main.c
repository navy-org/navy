#ifdef LIMINE_BOOTLOADER
#include <limine/base.h>
#endif

#ifdef ARCH_X86_64
#include <x86_64/base.h>
#endif

#include <copland/base.h>
#include <handover/handover.h>


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
    hardware_init(&handover);

    loop;
}