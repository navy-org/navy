#include <hal.h>
#include <io/stream.h>
#include <res.h>

#include "e9.h"
#include "gdt.h"

Stream hal_dbg_stream(void)
{
    return (Stream){
        .write = e9_puts,
    };
}

Res hal_setup(void)
{
    gdt_init();
    return ok$();
}