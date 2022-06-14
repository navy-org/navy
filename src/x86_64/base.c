#include "base.h"
#include "gdt.h"
#include "idt.h"

#include <copland/debug.h>

void hardware_init(MAYBE_UNUSED Handover *handover)
{
    gdt_init();
    idt_init();

    define_dbg_func(debug_interrupt);

    return;
}