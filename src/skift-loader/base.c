#define HANDOVER_INCLUDE_MACROS

#include <copland/base.h>
#include <skift-handover/includes/handover.h>
#include <kernel/abstraction.h>

HANDOVER(WITH_FB);

int _start(MAYBE_UNUSED uint64_t magic, MAYBE_UNUSED HandoverRecord *handover)
{
    Com debug = com_init(COM1);
    define_debug_out((Writer *) &debug);

    log$("Hello skift !");

    loop;
}