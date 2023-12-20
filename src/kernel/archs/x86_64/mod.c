#include <hal.h>
#include <io/stream.h>
#include <res.h>

#include "apic.h"
#include "e9.h"
#include "gdt.h"
#include "hpet.h"
#include "idt.h"
#include "paging.h"
#include "simd.h"
#include "syscall.h"

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
    try$(gdt_init_tss());
    syscall_init();
    acpi_init();
    try$(hpet_init());
    try$(apic_init());
    simd_init();

    return ok$();
}