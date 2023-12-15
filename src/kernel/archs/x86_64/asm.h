#pragma once

#include <stdint.h>

#define asm_read_cr(n, reg) asm volatile("mov %%cr" #n ", %0" \
                                         : "=r"(reg))

#define asm_write_cr(n, reg) asm volatile("mov %0, %%cr" #n \
                                          :                 \
                                          : "r"(reg))

enum msr_registers
{
    MSR_APIC = 0x1B,
    MSR_EFER = 0xC0000080,
    MSR_STAR = 0xC0000081,
    MSR_LSTAR = 0xC0000082,
    MSR_COMPAT_STAR = 0xC0000083,
    MSR_SYSCALL_FLAG_MASK = 0xC0000084,
    MSR_FS_BASE = 0xC0000100,
    MSR_GS_BASE = 0xC0000101,
    MSR_KERN_GS_BASE = 0xc0000102,
};

enum msr_star_reg
{
    STAR_KCODE_OFFSET = 32,
    STAR_UCODE_OFFSET = 48,
};

void asm_write_msr(uint64_t msr, uint64_t value);

uint64_t asm_read_msr(uint64_t msr);
