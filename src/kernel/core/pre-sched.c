#include <dbg/log.h>
#include <loader.h>
#include <specs/elf.h>
#include <string.h>

#include "hal.h"
#include "pmm.h"
#include "pre-sched.h"

#ifdef __ck_bits_64__
#    define Elf_Ehdr Elf64_Ehdr
#    define ELFCLASS ELFCLASS64
#    define Elf_Phdr Elf64_Phdr
#else
#    define Elf_Ehdr Elf32_Ehdr
#    define ELFCLASS ELFCLASS32
#    define Elf_Phdr Elf32_Phdr
#endif

static HalPage *sched_vspace;
static HalContext *sched_ctx;

static HalContext *kernel_ctx;
static bool need_switch = false;

Res load_scheduler(void)
{
    Module sched = loader_get_module("/bin/sched");
    if (sched.len == 0)
    {
        return err$(RES_NOENT);
    }

    try$(hal_space_create(&sched_vspace));

    Elf_Ehdr *hdr = (Elf_Ehdr *)sched.base;
    if (memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        return err$(RES_INVAL);
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS)
    {
        return err$(RES_INVAL);
    }

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(sched.base + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            log$("Mapping scheduler segment 0x%x -> 0x%x (len: %x)",
                 phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz, phdr->p_memsz);

            PmmObj paddr = pmm_alloc(align_up$(phdr->p_memsz, PMM_PAGE_SIZE));
            if (paddr.len == 0)
            {
                return err$(RES_NOMEM);
            }

            log$("Scheduler segment mapped to 0x%x", paddr.base);
            try$(hal_space_map(sched_vspace, phdr->p_vaddr, paddr.base, align_up$(phdr->p_memsz, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER | HAL_MEM_EXEC));

            memcpy((void *)hal_mmap_l2h(paddr.base), (void *)sched.base + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_l2h(paddr.base + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    sched_ctx = (HalContext *)try$(hal_context_create());
    kernel_ctx = (HalContext *)try$(hal_context_create());

    PmmObj sched_stack_obj = pmm_alloc(align_up$(STACK_SIZE, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
    if (sched_stack_obj.len == 0)
    {
        return err$(RES_NOMEM);
    }

    hal_space_map(sched_vspace, USER_STACK_BASE, sched_stack_obj.base, STACK_SIZE, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER);
    hal_context_start(sched_ctx, hdr->e_entry, USER_STACK_BASE, (SysArgs){0, 0, 0, 0, 0, 0});
    need_switch = true;

    return ok$();
}

void switch_to_scheduler(HalRegs *regs)
{
    if (need_switch)
    {
        hal_context_save(kernel_ctx, regs);

        hal_space_apply((HalPage *)sched_vspace);
        hal_context_restore(sched_ctx, regs);

        need_switch = false;

        return;
    }
}