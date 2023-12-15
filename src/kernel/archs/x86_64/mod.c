#include <hal.h>
#include <io/stream.h>
#include <res.h>

#include "e9.h"
#include "gdt.h"
#include "idt.h"
#include "paging.h"

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
    paging_init();
    acpi_init();
    return ok$();
}