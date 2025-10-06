#include <hal>
#include <kmalloc>
#include <pmm>
#include <traits>

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

void hal_setup(void)
{
    simd_init();
    gdt_init();
    idt_init();
    acpi_init();
    hpet_init();
    pmm_init();
    paging_init();
    kmalloc_init();
    gdt_init_tss();
    syscall_init();
    apic_init();
}
