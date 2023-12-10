#include <io/stream.h>

#include "e9.h"

Stream hal_dbg_stream(void)
{
    return (Stream){
        .write = e9_puts,
    };
}

void hal_setup(void)
{
}