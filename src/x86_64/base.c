#include "base.h"
#include "gdt.h"
#include "idt.h"


#include <copland/debug.h>

void hardware_init(MAYBE_UNUSED Handover const *handover)
{
    gdt_init();

#ifndef KERNEL_DEBUG
    idt_init();
#else 
    log$("Kernel debugging is enabled, no IDT for this build...")
#endif

    define_dbg_func(debug_interrupt);

    vmm_init(handover);
    acpi_init(handover);
    pic_mask_interrupts();
    apic_init();

    return;
}