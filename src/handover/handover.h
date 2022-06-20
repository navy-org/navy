#pragma once 

#include <copland/base.h>

#define LIMIT_ENTRIES 64

#define FOREACH_MEMMAP_TYPE(FUNC)         \
    FUNC(MEMMAP_USABLE)                   \
    FUNC(MEMMAP_RESERVED)                 \
    FUNC(MEMMAP_ACPI_RECLAIMABLE)         \
    FUNC(MEMMAP_ACPI_NVS)                 \
    FUNC(MEMMAP_BAD_MEMORY)               \
    FUNC(MEMMAP_BOOTLOADER_RECLAIMABLE)   \
    FUNC(MEMMAP_KERNEL_AND_MODULES)       \
    FUNC(MEMMAP_FRAMEBUFFER)

typedef enum mmap_type 
{
    FOREACH_MEMMAP_TYPE(GENERATE_ENUM)
} MmapType;

[[gnu::used]] static char const *mmap_type_str[] = {
    FOREACH_MEMMAP_TYPE(GENERATE_STRING)
};

typedef struct 
{
    Str name;
    Range range;
} Module;

typedef struct 
{
    Range range;
    MmapType type;
} MmapEntry;

typedef void (*CpuGoto)(void *cpu);

typedef struct _SmpEntry
{
    uint64_t core_count;
    uint32_t bsp_lapic_id;
    void **cpus;
    void (*core_goto)(void **cpus, uint32_t id, CpuGoto addr);
} SmpEntry;

typedef struct 
{
    size_t mmap_count;
    size_t module_count;

    uint64_t hhdm_offset;

    uint64_t kernel_pbase;
    uint64_t kernel_vbase;
    uint64_t rsdp_address;

    SmpEntry smp;
    MmapEntry mmaps[LIMIT_ENTRIES];
    Module modules[LIMIT_ENTRIES];
} Handover;