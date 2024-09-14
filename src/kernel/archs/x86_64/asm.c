#include "asm.h"

void out8(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(value), "Nd"(port));
}

uint8_t in8(uint16_t port)
{
    uint8_t value;
    asm volatile("inb %1, %0"
                 : "=a"(value)
                 : "Nd"(port));
    return value;
}

void _hal_disable_interrupts(void)
{
    asm volatile("cli");
}

void _hal_enable_interrupts(void)
{
    asm volatile("sti");
}

void hal_pause(void)
{
    asm volatile("pause");
}

void hal_panic(void)
{
    asm volatile("int $1");
}

void asm_write_msr(uint64_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile("wrmsr" ::"c"((uint64_t)msr), "a"(low), "d"(high));
}

uint64_t asm_read_msr(uint64_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"((uint64_t)msr));
    return ((uint64_t)high << 32) | low;
}

void asm_write_xcr(uint32_t xcr, uint64_t value)
{
    uint32_t edx = value >> 32;
    uint32_t eax = (uint32_t)value;
    asm volatile("xsetbv"
                 :
                 : "a"(eax), "d"(edx), "c"(xcr)
                 : "memory");
}

uint64_t asm_read_xcr(uint32_t xcr)
{
    uint32_t eax, edx;
    asm volatile("xgetbv"
                 : "=a"(eax), "=d"(edx)
                 : "c"(xcr)
                 : "memory");

    return eax | ((uint64_t)edx << 32);
}

void asm_xsave(uint8_t *region)
{
    asm volatile("xsave %0" ::"m"(*region), "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                 : "memory");
}

void asm_xrstor(uint8_t *region)
{
    asm volatile("xrstor %0" ::"m"(*region), "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                 : "memory");
}

void asm_fxsave(void *region)
{
    asm volatile("fxsave (%0)" ::"a"(region));
}

void asm_fxrstor(void *region)
{
    asm volatile("fxrstor (%0)" ::"a"(region));
}
