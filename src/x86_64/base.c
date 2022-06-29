#include "base.h"

#include <kernel/cpu.h>
#include <copland/debug.h>

void hardware_init(Handover const *handover)
{
    gdt_init();

#ifndef __osdk_debug_mode__
    idt_init();
#else 
    log$("Kernel debugging is enabled, no IDT for this build...")
#endif

    define_dbg_func(debug_interrupt);
    
    vmm_init(handover);
    acpi_init(handover);
    pic_mask_interrupts();
    pic_disable();
    pit_init();
    apic_init(handover);
    cpu_preinit(handover);
    cpu_init();

    return;
}