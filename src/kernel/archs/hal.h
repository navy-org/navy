#pragma once

#include <io/stream.h>

Res hal_setup(void);

/* --- Structs ------------------------------------------------------------  */

typedef struct _HalRegs HalRegs;

/* --- Assembly function --------------------------------------------------- */

void hal_disable_interrupts(void);

void hal_enable_interrupts(void);

void hal_pause(void);

void hal_panic(void);

/* --- I/O ----------------------------------------------------------------  */

Stream hal_dbg_stream(void);
