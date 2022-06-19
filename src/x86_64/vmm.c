#include "base.h"

#include <copland/base.h>

#include <kernel/pmm.h>
#include <kernel/const.h>
#include <kernel/spinlock.h>

static Pml *kernel_pml;
static uint64_t hhdm_offset;
static uint32_t lock;
static uint32_t limit_pml = 4;

static Range vmm_get_pml_alloc(Pml *pml, size_t index, bool is_user)
{
    PmlEntry entry = pml->entries[index];

    if (entry.present)
    {
        Range pml_range = {(entry.physical << 12) + hhdm_offset, PAGE_SIZE};
        return pml_range;
    }
    else  
    {
        Range target_range = UNWRAP_OR_PANIC(pmm_alloc(PAGE_SIZE), "Out of memory");
        memset((void *) (target_range.base + hhdm_offset), 0, PAGE_SIZE);
        pml->entries[index] = pml_make_entry(target_range.base, is_user);

        return (Range) {target_range.base + hhdm_offset, PAGE_SIZE};
    }
}

static void vmm_map_page(Pml *pml, uintptr_t virt, uintptr_t phys, bool is_user)
{
    lock$(lock);

    Pml *last_entry = pml;

    for (size_t i = limit_pml - 1; i > 0; i--)
    {
        Range pml_range = vmm_get_pml_alloc(last_entry, PMLX_GET_INDEX(virt, i), true);
        last_entry = (Pml *) pml_range.base;
    }

    last_entry->entries[PMLX_GET_INDEX(virt, 0)] = pml_make_entry(phys, is_user);

    unlock$(lock);
}

void vmm_map_range(Pml *pml, Range virt, Range phys, bool is_user)
{

    if(virt.length != phys.length)
    {
        panic$("virt.length {} != phys.length {}", virt.length, phys.length);
    }

    if (virt.base % PAGE_SIZE != 0 || virt.length % PAGE_SIZE != 0 
        || phys.base % PAGE_SIZE != 0 || phys.length % PAGE_SIZE != 0)
    {
        panic$("Couldn't map unaligned ranges");
    }

    size_t physaddr;
    size_t virtaddr;

    for (size_t i = 0; i < (virt.length / PAGE_SIZE); i++)
    {
        physaddr = i * PAGE_SIZE + align_down$(phys.base, PAGE_SIZE);
        virtaddr = i * PAGE_SIZE + align_down$(virt.base, PAGE_SIZE);

        vmm_map_page(pml, virtaddr, physaddr, is_user);
    }
}

static void vmm_switch_space(Pml *space)
{
    lock$(lock);
    write_cr3((uintptr_t) (space) - hhdm_offset);
    unlock$(lock);
}

void vmm_init(const Handover *handover)
{
    hhdm_offset = handover->hhdm_offset;

    kernel_pml = (Pml *) (UNWRAP(pmm_alloc(1)).base + hhdm_offset);
    memset(kernel_pml, 0, PAGE_SIZE);

    vmm_map_range(kernel_pml,
        (Range) {
            .base = hhdm_offset,
            .length = gib$(4)
        }, 
        (Range) {
            .base = 0,
            .length = gib$(4)
        }, false);

    vmm_map_range(kernel_pml, 
        (Range) {
            .base = handover->kernel_vbase,
            .length = gib$(4)
        }, 
        (Range){
            .base = handover->kernel_pbase,
            .length = gib$(4)
        }, false);

    for (size_t i = 0; i < handover->mmap_count; i++)
    {
        MmapEntry entry = handover->mmaps[i];

        vmm_map_range(kernel_pml,
            (Range) {
                .base = entry.range.base + hhdm_offset,
                .length = entry.range.length
            }, entry.range, false);
    }

    vmm_switch_space(kernel_pml);
}