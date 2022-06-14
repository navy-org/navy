#ifdef LIMINE_BOOTLOADER
#include <limine/base.h>
#endif

#include <x86_64/com.h>
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

    loop;
}