#pragma once 

#include <copland/macro.h>
#include <stdint.h>

static inline uint8_t asm_in8(uint16_t port)
{
    uint8_t data;
    asm volatile("inb %1, %0"
                 : "=a"(data)
                 : "d"(port));
    return data;
}

static inline void asm_out8(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(data), "d"(port));
}

static inline uintptr_t read_cr4(void)
{
    uintptr_t value;
    __asm__ volatile("mov %%cr4, %0" : "=r"(value));

    return value;
}

static inline uintptr_t read_cr3(void)
{
    uintptr_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));

    return value;
}

static inline uintptr_t read_cr2(void)
{
    uintptr_t value;
    __asm__ volatile("mov %%cr2, %0" : "=r"(value));

    return value;
}

static inline uintptr_t read_cr0(void)
{
    uintptr_t value;
    __asm__ volatile("mov %%cr0, %0" : "=r"(value));

    return value;
}

static inline void write_cr4(uintptr_t val)
{
    asm volatile("mov %0, %%cr4" ::"a"(val));
}

static inline void write_cr3(uintptr_t val)
{
    asm volatile("mov %0, %%cr3" ::"a"(val));
}

static inline void write_cr0(uintptr_t val)
{
    asm volatile("mov %0, %%cr0" ::"a"(val));
}

MAYBE_UNUSED static void asm_invlpg(uintptr_t addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr): "memory");
}


// Lots of info here https://en.wikipedia.org/wiki/Control_register

enum cr0_bits 
{
    CR0_PE = (1 << 0),
    CR0_MP = (1 << 1),
    CR0_EM = (1 << 2),
    CR0_TS = (1 << 3),
    CR0_ET = (1 << 4),
    CR0_NE = (1 << 5),
    CR0_WP = (1 << 16),
    CR0_AM = (1 << 18),
    CR0_NW = (1 << 29),
    CR0_CD = (1 << 30),
    CR0_PG = (1 << 31)
};

enum cr4_bits 
{
    CR4_VME = (1 << 0),
    CR4_PVI = (1 << 1),
    CR4_TSD = (1 << 2),
    CR4_DE = (1 << 3),
    CR4_PSE = (1 << 4),
    CR4_PAE = (1 << 5),
    CR4_MCE = (1 << 6),
    CR4_PGE = (1 << 7),
    CR4_PCE = (1 << 8),
    CR4_OSFXSR = (1 << 9),
    CR4_OSXMMEXCPT = (1 << 10),
    CR4_UMIP = (1 << 11),
    CR4_LA57 = (1 << 12),
    CR4_VMXE = (1 << 13),
    CR4_SMXE = (1 << 14),
    CR4_FSGSBASE = (1 << 16),
    CR4_PCIDE = (1 << 17),
    CR4_OSXSAVE = (1 << 18),
    CR4_SMEP = (1 << 20),
    CR4_SMAP = (1 << 21),
    CR4_PKE = (1 << 22),
    CR4_CET = (1 << 23),
    CR4_PKS = (1 << 24)
};

enum xcr0_bits 
{
    XCR0_X87 = (1 << 0),
    XCR0_SSE = (1 << 1),
    XCR0_AVX = (1 << 2),
    XCR0_BNDREG = (1 << 3),
    XCR0_BNDCSR = (1 << 4),
    XCR0_OPMASK = (1 << 5),
    XCR0_ZMM_HI256 = (1 << 6),
    XCR0_HI16_ZMM = (1 << 7),
    XCR0_PKRU = (1 << 9),
    XCR0_CET_USER_STATE = (1 << 11),
    XCR0_CET_SUPERVISOR = (1 << 12)
};



static inline void write_xcr(uint32_t i, uint64_t value)
{
    asm volatile ("xsetbv"
        :
        : "a" ((uint32_t) value), "d" (value >> 32), "c" (i));
}

static inline void asm_xsave(uint8_t *region)
{
    asm volatile("xsave %0" ::"m"(*region), "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                 : "memory");
}

static inline void asm_fxsave(void *region)
{
    asm volatile("fxsave (%0)" ::"a"(region));
}

static inline void asm_fxrstor(void *region)
{
    asm volatile("fxrstor (%0)" ::"a"(region));
}

static inline void asm_xrstor(uint8_t *region)
{
    asm volatile("xrstor %0" ::"m"(*region), "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                 : "memory");
}

static inline void io_wait(void)
{
    __asm__ volatile ("jmp 1f\n\t" "1:jmp 2f\n\t" "2:");
}


enum msr_star_reg
{
    STAR_KCODE_OFFSET = 32,
    STAR_UCODE_OFFSET = 48,
};

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


static inline void asm_write_msr(enum msr_registers msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile("wrmsr" :: "c"((uint64_t)msr), "a"(low), "d"(high));
}

static inline uint64_t asm_read_msr(enum msr_registers msr)
{
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"((uint64_t)msr));
    return ((uint64_t) high << 32) | low;
}

static inline void hlt(void) 
{
    asm ("hlt");
}

static inline void cli(void)
{
    asm ("cli");
}

static inline void sti(void)
{
    asm ("sti");
}

static inline void debug_interrupt(void)
{
    asm ("int $1");
}