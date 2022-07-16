#include "base.h"

#include <kernel/cpu.h>
#include <copland/debug.h>

void hardware_init(Handover const *handover)
{
    log$("Initializing hardware...");
    gdt_init();
    idt_init();

    define_dbg_func(debug_interrupt);
    
    vmm_init(handover);
    acpi_init(handover);
    pic_mask_interrupts();
    pic_disable();
    pit_init();
    apic_init(handover);
    cpu_preinit(handover);
    cpu_init();

    log$("Hardware initialization complete.");

    return;
}