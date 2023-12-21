#include <dbg/log.h>
#include <handover/handover.h>
#include <stdint.h>

_Noreturn int _start(uintptr_t handover_addr)
{
    HandoverPayload *handover = (HandoverPayload *)handover_addr;

    if (handover->magic != HANDOVER_MAGIC)
    {
        error$("Invalid handover magic");
        for (;;);
    }

    log$("Hello, world!");
    for (;;)
        ;
}