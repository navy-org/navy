#include <logging>
#include <pmm>
#include <stdint.h>
#include <string.h>

#include "asm.h"
#include "cpu.h"
#include "cpuid.h"
#include "simd.h"

static _Alignas(PMM_PAGE_SIZE) uint8_t simd_initial_context[PMM_PAGE_SIZE] = {};

void simd_init(void)
{
    uint64_t cr0;
    uint64_t cr4;

    uint64_t xcr0 = XCR0_x87 | XCR0_SSE;
    bool xcr0_compat = true;

    asm_read_cr(0, cr0);
    cr0 &= ~CR0_EMULATION;
    cr0 |= CR0_MONITOR_COPROCESSOR;
    cr0 |= CR0_NUMERIC_ERROR;
    asm_write_cr(0, cr0);

    asm_read_cr(4, cr4);
    cr4 |= CR4_OSFXSR;
    cr4 |= CR4_OSXMMEXCPT_ENABLE;

    if (cpuid_has_xsave())
    {
        log$("XSAVE is supported");
        cr4 |= CR4_OSXSAVE_ENABLE;
    }
    else
    {
        xcr0_compat = false;
    }

    asm_write_cr(4, cr4);

    if (cpuid_has_avx())
    {
        log$("AVX is supported");
        xcr0 |= XCR0_AVX;
    }

    if (cpuid_has_avx512())
    {
        log$("AVX512 is supported");
        xcr0 |= XCR0_OPMASK;
        xcr0 |= XCR0_ZMM_Hi256;
        xcr0 |= XCR0_Hi16_ZMM;
    }

    if (xcr0_compat)
    {
        asm_write_xcr(0, xcr0);
    }

    asm volatile("fninit");
    simd_context_save(simd_initial_context);

    log$("SIMD initialized");
}

void simd_context_save(void *ptr)
{
    if (cpuid_has_xsave())
    {
        asm_xsave(ptr);
    }
    else
    {
        asm_fxsave(ptr);
    }
}

size_t simd_context_size(void)
{
    if (cpuid_has_xsave())
    {
        return cpuid_xsave_size();
    }
    else
    {
        return 512;
    }
}

void simd_context_load(void *ptr)
{
    if (cpuid_has_xsave())
    {
        asm_xrstor(ptr);
    }
    else
    {
        asm_fxrstor(ptr);
    }
}

void simd_context_init(void *ptr)
{
    memcpy(ptr, simd_initial_context, simd_context_size());
}