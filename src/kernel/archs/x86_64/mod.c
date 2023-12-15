#include <hal.h>
#include <io/stream.h>
#include <res.h>

#include "e9.h"
#include "gdt.h"
#include "idt.h"
#include "paging.h"
#include "hpet.h"
#include "apic.h"
#include "simd.h"

Stream hal_dbg_stream(void)
{
    return (Stream){
        .write = e9_puts,
    };
}

Res hal_setup(void)
{
    gdt_init();
    idt_init();
    try$(paging_init());
    acpi_init();
    try$(hpet_init());
    try$(apic_init());
    simd_init();

    return ok$();
}