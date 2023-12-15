#pragma once

#include <io/stream.h>
#include <stdint.h>

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

/* --- Memory mapping ------------------------------------------------------ */

typedef struct _page HalPage;

typedef enum
{
    HAL_MEM_NONE = 0,
    HAL_MEM_READ = 1 << 0,
    HAL_MEM_WRITE = 1 << 1,
    HAL_MEM_EXEC = 1 << 2,
    HAL_MEM_USER = 1 << 3,
    HAL_MEM_HUGE = 1 << 4,
} HalMemFlags;

uintptr_t hal_mmap_l2h(uintptr_t addr);

uintptr_t hal_mmap_h2l(uintptr_t addr);

Res hal_space_map(HalPage *space, uintptr_t virt, uintptr_t phys, size_t len, uint8_t flags);

void hal_space_apply(HalPage *space);

/* --- Arch Specific ------------------------------------------------------- */

#ifdef __ck_arch_x86_64__
#    include <x86_64/acpi.h>

Rsdp *hal_acpi_rsdp(void);

#endif
