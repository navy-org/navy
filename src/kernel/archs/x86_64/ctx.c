#include <errno.h>
#include <hal>
#include <kmalloc>
#include <logger>
#include <pmm>
#include <string.h>

#include "cpu.h"
#include "ctx.h"
#include "gdt.h"
#include "simd.h"
#include "syscall.h"

HalContext *hal_context_create(void)
{
    Alloc alloc = kmalloc_acquire();

    HalContext *self = (HalContext *)alloc.calloc(1, sizeof(HalContext) + simd_context_size());
    if (IS_ERR(self))
    {
        return self;
    }

    simd_context_init(self);
    return self;
}

void hal_context_destroy(HalContext *self)
{
    Alloc alloc = kmalloc_acquire();
    alloc.free(self);
}

long hal_context_start(HalContext *self, uintptr_t ip, uintptr_t sp)
{
    self->regs.rip = ip;
    self->regs.rflags = RFLAGS_INTERRUPT_ENABLE | RFLAGS_RESERVED1;

    self->regs.cs = (GDT_USER_CODE * 8) | 3;
    self->regs.ss = (GDT_USER_DATA * 8) | 3;
    self->regs.rsp = sp;
    self->regs.rbp = 0;

    PhysObj kStackObj = pmm_alloc(align_up$(STACK_SIZE, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
    if (kStackObj.base == 0)
    {
        return -ENOMEM;
    }

    self->syscall_kernel_stack = hal_mmap_l2h(kStackObj.base) + KERNEL_STACK_SIZE;
    memset((void *)self->syscall_kernel_stack, 0, KERNEL_STACK_SIZE);

    return 0;
}

void hal_context_save(HalContext *self, HalRegs *regs)
{
    simd_context_save(self->simd);
    self->regs = *regs;
}

void hal_context_restore(HalContext *self, HalRegs *regs)
{
    *regs = self->regs;
    simd_context_load(self->simd);
    syscall_set_gs((uintptr_t)self);
}
