#include <hal>
#include <handover>
#include <logger>
#include <pmm>
#include <result>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils.h>

#include "asm.h"
#include "cpuid.h"
#include "paging.h"

static size_t page_size = mib$(2);
static uintptr_t *pml4 = NULL;

extern char text_start_addr[];
extern char text_end_addr[];
extern char rodata_start_addr[];
extern char rodata_end_addr[];
extern char data_start_addr[];
extern char data_end_addr[];

static int64_t transform_flags(HalMemFlags flags)
{
    int64_t ret_flags = PAGE_NO_EXECUTE | PAGE_PRESENT;

    if (flags & HAL_MEM_READ)
    {
    }

    if (flags & HAL_MEM_NONE)
    {
        ret_flags &= ~PAGE_PRESENT;
    }

    if (flags & HAL_MEM_WRITE)
    {
        ret_flags |= PAGE_WRITABLE;
    }

    if (flags & HAL_MEM_EXEC)
    {
        ret_flags &= ~PAGE_NO_EXECUTE;
    }

    if (flags & HAL_MEM_USER)
    {
        ret_flags |= PAGE_USER;
    }

    if (flags & HAL_MEM_HUGE)
    {
        ret_flags |= PAGE_HUGE;
    }

    return ret_flags;
}

static Res paging_get_pml_alloc(uintptr_t *pml, size_t index, bool alloc)
{
    if ((pml[index] & PAGE_PRESENT))
    {
        return uok$(hal_mmap_l2h(PAGE_GET_PHYS(pml[index])));
    }
    else if (alloc)
    {
        PhysObj obj = pmm_alloc(1);
        uintptr_t ptr_hddm = hal_mmap_l2h(obj.base);
        memset((void *)ptr_hddm, 0, obj.len);

        pml[index] = obj.base | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

        return uok$(ptr_hddm);
    }

    return err$(RES_NOMEM);
}

static Res kmmap_page(uintptr_t *pml, uint64_t virt, uint64_t phys, int64_t flags)
{
    if (phys % PMM_PAGE_SIZE != 0 || virt % PMM_PAGE_SIZE != 0)
    {
        return err$(RES_BADALIGN);
    }

    size_t pml1_entry = PMLX_GET_INDEX(virt, 0);
    size_t pml2_entry = PMLX_GET_INDEX(virt, 1);
    size_t pml3_entry = PMLX_GET_INDEX(virt, 2);
    size_t pml4_entry = PMLX_GET_INDEX(virt, 3);

    uintptr_t *pml3 = (uintptr_t *)try$(paging_get_pml_alloc(pml, pml4_entry, true));

    if (page_size == gib$(1) && flags & PAGE_HUGE)
    {
        pml3[pml3_entry] = phys | flags;
        return ok$();
    }

    uintptr_t *pml2 = (uintptr_t *)try$(paging_get_pml_alloc(pml3, pml3_entry, true));

    if (flags & PAGE_HUGE)
    {
        pml2[pml2_entry] = phys | flags;
        return ok$();
    }

    uintptr_t *pml1 = (uintptr_t *)try$(paging_get_pml_alloc(pml2, pml2_entry, true));

    pml1[pml1_entry] = phys | flags;
    return ok$();
}

static Res kmmap_section(uintptr_t start, uintptr_t end, uint8_t flags)
{
    KernelMmap kaddr = loader_get_kernel_mmap();
    int64_t flags_arch = transform_flags(flags);
    size_t end_loop = align_up$(end, PMM_PAGE_SIZE);

    for (size_t i = align_down$(start, PMM_PAGE_SIZE); i < end_loop; i += PMM_PAGE_SIZE)
    {
        uintptr_t phys = i - kaddr.virt + kaddr.phys;
        try$(kmmap_page(pml4, i, phys, flags_arch));
    }

    return ok$();
}

Res hal_space_map(HalPage *self, uintptr_t virt, uintptr_t phys, size_t len, uint8_t flags)
{
    if (phys % PMM_PAGE_SIZE != 0 || virt % PMM_PAGE_SIZE != 0 || len % PMM_PAGE_SIZE != 0)
    {
        return err$(RES_BADALIGN);
    }

    int64_t flags_arch = transform_flags(flags);
    const size_t map_psize = flags & HAL_MEM_HUGE ? page_size : PMM_PAGE_SIZE;

    size_t end = align_up$(len, map_psize);
    size_t aligned_virt = align_down$(virt, map_psize);
    size_t aligned_phys = align_down$(phys, map_psize);

    for (size_t i = 0; i < end; i += map_psize)
    {
        try$(kmmap_page((uintptr_t *)self, aligned_virt + i, aligned_phys + i, flags_arch));
    }

    return ok$();
}

Res hal_space_unmap(HalPage *space, uintptr_t virt, size_t len)
{
    if (virt % PMM_PAGE_SIZE != 0 || len % PMM_PAGE_SIZE != 0)
    {
        return err$(RES_BADALIGN);
    }

    // TODO: Huge pages ?

    for (size_t i = virt; i < len; i += PMM_PAGE_SIZE)
    {
        size_t pml1_entry = PMLX_GET_INDEX(i, 0);
        size_t pml2_entry = PMLX_GET_INDEX(i, 1);
        size_t pml3_entry = PMLX_GET_INDEX(i, 2);
        size_t pml4_entry = PMLX_GET_INDEX(i, 3);

        uintptr_t *pml3 = (uintptr_t *)try$(paging_get_pml_alloc((uintptr_t *)space, pml4_entry, false));
        uintptr_t *pml2 = (uintptr_t *)try$(paging_get_pml_alloc(pml3, pml3_entry, false));
        uintptr_t *pml1 = (uintptr_t *)try$(paging_get_pml_alloc(pml2, pml2_entry, false));

        pml1[pml1_entry] = 0;
    }

    return ok$();
}

Res paging_init(void)
{
    PhysObj obj = pmm_alloc(1);
    if (obj.base == 0)
    {
        return err$(RES_NOMEM);
    }

    log$("PML4: %p", obj.base);
    pml4 = (uintptr_t *)hal_mmap_l2h((uintptr_t)obj.base);

    memset((void *)pml4, 0, obj.len);

    if (cpuid_has_1gb_pages())
    {
        log$("1GB pages are supported");
        page_size = gib$(1);
    }
    else
    {
        log$("1GB pages are not supported, defaulting to 2MB pages");
        page_size = mib$(2);
    }

    kmmap_section((uintptr_t)text_start_addr, (uintptr_t)text_end_addr, HAL_MEM_READ | HAL_MEM_EXEC);
    kmmap_section((uintptr_t)rodata_start_addr, (uintptr_t)rodata_end_addr, HAL_MEM_READ);
    kmmap_section((uintptr_t)data_start_addr, (uintptr_t)data_end_addr, HAL_MEM_READ | HAL_MEM_WRITE);

    log$("Kernel sections mapped");

    size_t end = max$(gib$(4), pmm_available_pages() * PMM_PAGE_SIZE);
    uint64_t flags = transform_flags(HAL_MEM_WRITE | HAL_MEM_READ | HAL_MEM_HUGE);

    for (size_t i = page_size; i < end; i += page_size)
    {
        try$(kmmap_page(pml4, hal_mmap_l2h(i), i, flags));
    }

    HandoverPayload *hand = handover();
    HandoverRecord rec;

    handover_foreach_record(hand, rec)
    {
        if (rec.tag != HANDOVER_FB)
        {
            try$(hal_space_map((HalPage *)pml4,
                               align_down$(hal_mmap_l2h(rec.start), PMM_PAGE_SIZE),
                               align_down$(rec.start, PMM_PAGE_SIZE),
                               align_up$(rec.size, PMM_PAGE_SIZE),
                               HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_HUGE));
        }
    }

    log$("Memory mapped");
    hal_space_apply((HalPage *)pml4);
    log$("Space applied");

    return ok$();
}

void hal_space_apply(HalPage *space)
{
    asm_write_cr(3, hal_mmap_h2l((uintptr_t)space));
}

Res hal_space_create(HalPage **self)
{

    PhysObj obj = pmm_alloc(1);
    uintptr_t *space = (uintptr_t *)hal_mmap_l2h(obj.base);
    memset((void *)space, 0, obj.len);

    if (obj.base == 0)
    {
        return err$(RES_NOMEM);
    }

    memset((void *)space, 0, PMM_PAGE_SIZE);

    for (size_t i = 255; i < 512; i++)
    {
        space[i] = pml4[i];
    }

    *self = (HalPage *)space;
    return ok$();
}

HalPage *hal_space_kernel(void)
{
    return (HalPage *)pml4;
}

Res hal_virt2phys(HalPage *space, uintptr_t virt)
{
    size_t pml1_entry = PMLX_GET_INDEX(virt, 0);
    size_t pml2_entry = PMLX_GET_INDEX(virt, 1);
    size_t pml3_entry = PMLX_GET_INDEX(virt, 2);
    size_t pml4_entry = PMLX_GET_INDEX(virt, 3);

    uintptr_t *pml3 = (uintptr_t *)try$(paging_get_pml_alloc((uintptr_t *)space, pml4_entry, false));
    uintptr_t *pml2 = (uintptr_t *)try$(paging_get_pml_alloc(pml3, pml3_entry, false));
    uintptr_t *pml1 = (uintptr_t *)try$(paging_get_pml_alloc(pml2, pml2_entry, false));

    if (!(pml1[pml1_entry] & PAGE_PRESENT))
    {
        return err$(RES_NOENT);
    }

    return uok$(PAGE_GET_PHYS(pml1[pml1_entry]));
}
