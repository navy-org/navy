#include "base.h"
#include "gdt.h"

void hardware_init(MAYBE_UNUSED Handover *handover)
{
    gdt_init();
    return;
}