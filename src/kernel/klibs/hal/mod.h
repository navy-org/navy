#pragma once

#include <stdint.h>
#include <traits>

#ifdef __ck_arch_x86_64__
#    include <x86_64/acpi.h>
#    include <x86_64/ctx.h>
#    include <x86_64/regs.h>
#endif

Res hal_setup(void);

/* --- Structs ------------------------------------------------------------  */

typedef struct _HalRegs HalRegs;

/* --- Assembly function --------------------------------------------------- */

void _hal_disable_interrupts(void);

void hal_disable_interrupts(void);

void _hal_enable_interrupts(void);

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

Res hal_space_unmap(HalPage *space, uintptr_t virt, size_t len);

void hal_space_apply(HalPage *space);

Res hal_space_create(HalPage **self);

HalPage *hal_space_kernel(void);

Res hal_virt2phys(HalPage *space, uintptr_t virt);

/* --- Arch Specific ------------------------------------------------------- */

#ifdef __ck_arch_x86_64__
Rsdp *hal_acpi_rsdp(void);
#endif

typedef struct _HalContext HalContext;

Res hal_context_create(void);

Res hal_context_start(HalContext *self, uintptr_t ip, uintptr_t sp);

void hal_context_destroy(HalContext *self);

void hal_context_save(HalContext *self, HalRegs *regs);

void hal_context_restore(HalContext *self, HalRegs *regs);

void hal_regs_dump(HalRegs const *regs);

/* --- Syscalls ------------------------------------------------------------ */

typedef uintptr_t SysArg;

typedef enum : size_t
{
    __SYSCALL_LENGTH,
} Syscalls;

typedef struct
{
    SysArg arg1;
    SysArg arg2;
    SysArg arg3;
    SysArg arg4;
    SysArg arg5;
    SysArg arg6;
} SysArgs;

Res _syscall_handler(Syscalls no, SysArgs args);
