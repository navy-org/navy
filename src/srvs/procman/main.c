#include <dbg/log.h>
#include <handover/handover.h>

Res main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        error$("no handover payload was provided");
        return err$(RES_INVAL);
    }

    HandoverPayload *handover = (HandoverPayload *)argv[1];

    if (handover->magic != HANDOVER_MAGIC)
    {
        error$("Invalid handover magic");
        return err$(RES_INVAL);
    }

    log$("Hello from procman!");

    return ok$();
}