#pragma once 

#include <handover/handover.h>

#define PMLX_GET_INDEX(addr, level)                                                                \
    (((uint64_t) addr & ((uint64_t) 0x1ff << (12 + level * 9))) >> (12 + level * 9))

typedef struct PACKED
{
    union 
    {
        uint64_t raw;

        struct PACKED 
        {
            bool present : 1;
            bool read_write : 1;
            bool user : 1;
            bool caching : 1;
            bool caching_disable : 1;
            bool accessed : 1;
            bool dirty : 1;
            bool huge_page : 1;
            bool global_page : 1;
            uint8_t _available : 3;
            uint64_t physical : 52;
        };
    };
} PmlEntry;

typedef struct PACKED
{
    PmlEntry entries[512];
} Pml;

static inline PmlEntry pml_make_entry(uintptr_t physical, bool is_user)
{
    return (PmlEntry){
        .physical = physical >> 12,
        .user = is_user,
        .read_write = true,
        .present = true,
        .caching = false,
        .caching_disable = false,
        .accessed = false,
        .dirty = false,
        .huge_page = false,
        .global_page = false,
        ._available = 0,
    };
}

typedef Option(Pml *) PmlOption;

void vmm_map_range(Pml *pml, Range virt, Range phys, bool is_user);
void vmm_init(Handover const *handover);

PmlOption vmm_create_space(void);
Pml *vmm_get_kernel_space(void);
Pml *vmm_get_current_space(void);
void vmm_switch_space(Pml *space);