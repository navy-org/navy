#include <hal>
#include <handover>
#include <logging>
#include <pmm>
#include <sched>
#include <specs/elf.h>
#include <sync>

#include "mod.h"

Res elfloader_instantiate(char const *filename)
{
    HandoverRecord file = handover_file_find(handover(), filename);
    if (file.size == 0)
    {
        return err$(RES_NOENT);
    }

    Elf64_Ehdr *hdr = (Elf64_Ehdr *)file.start;
    if (memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        return err$(RES_INVAL);
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        return err$(RES_INVAL);
    }

    HalPage *vspace;
    try$(hal_space_create(&vspace));

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf64_Phdr *phdr = (Elf64_Phdr *)(file.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            log$("Mapping %s segment 0x%x -> 0x%x (len: %x)",
                 filename, phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz, phdr->p_memsz);

            PmmObj paddr = pmm_alloc(align_up$(phdr->p_memsz, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
            if (paddr.len == 0)
            {
                return err$(RES_NOMEM);
            }

            log$("%s segment mapped to 0x%x", filename, paddr.base);
            try$(hal_space_map(vspace, phdr->p_vaddr, paddr.base, align_up$(phdr->p_memsz, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER | HAL_MEM_EXEC));

            memcpy((void *)hal_mmap_l2h(paddr.base), (void *)file.start + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_l2h(paddr.base + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    try$(sched_add((Task *)try$(task_new(filename, uok$(vspace), uok$(hdr->e_entry)))));

    hal_enable_interrupts();

    return ok$();
}