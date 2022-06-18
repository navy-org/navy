#include "apic.h"

#include <copland/base.h>

void apic_init(void)
{
    Apic *apic = (Apic *) UNWRAP(acpi_parse(str$("APIC")));
    log$("{a}", (uintptr_t) apic->lapic_address);
}