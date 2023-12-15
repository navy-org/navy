#pragma once

#include <stddef.h>
#include <stdint.h>

#define CPUID_EXTENDED_LEAF                   (0x80000001)
#define CPUID_EXFEATURE_PDPE1GB               (1 << 26)
#define CPUID_XSAVE_SUPPORT                   (1 << 26)
#define CPUID_AVX_SUPPORT                     (1 << 28)
#define CPUID_AVX512_SUPPORT                  (1 << 16)
#define CPUID_FEATURE_IDENTIFIER              (0x1)
#define CPUID_EXTENDED_FEATURE_IDENTIFIER     (0x7)
#define CPUID_PROC_EXTENDED_STATE_ENUMERATION (0xD)
typedef struct
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    bool success;
} CpuidResult;

bool cpuid_has_1gb_pages(void);
bool cpuid_has_xsave(void);
bool cpuid_has_avx(void);
bool cpuid_has_avx512(void);
size_t cpuid_xsave_size(void);