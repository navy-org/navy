#include <errno.h>
#include <hal>
#include <handover>
#include <logger>
#include <pmm>
#include <sched>
#include <specs/elf.h>
#include <sync>

#include "mod.h"

long elfloader_instantiate(char const *filename)
{
    long err;
    HandoverRecord file = handover_file_find(handover(), filename);
    if (file.size == 0)
    {
        return -ENOENT;
    }

    Elf64_Ehdr *hdr = (Elf64_Ehdr *)file.start;
    if (memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        return -EINVAL;
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        return -EINVAL;
    }

    HalPage *vspace;
    err = hal_space_create(&vspace);
    if (IS_ERR_VALUE(err))
    {
        return err;
    }

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf64_Phdr *phdr = (Elf64_Phdr *)(file.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            log$("Mapping %s segment 0x%x -> 0x%x (len: %x)",
                 filename, phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz, phdr->p_memsz);

            PhysObj paddr = pmm_alloc(align_up$(phdr->p_memsz, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
            if (paddr.len == 0)
            {
                return -ENOMEM;
            }

            log$("%s segment mapped to 0x%x", filename, paddr.base);
            err = hal_space_map(vspace, phdr->p_vaddr, paddr.base, align_up$(phdr->p_memsz, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER | HAL_MEM_EXEC);

            if (IS_ERR_VALUE(err))
            {
                return err;
            }

            memcpy((void *)hal_mmap_l2h(paddr.base), (void *)file.start + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_l2h(paddr.base + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    Task *task = task_new(filename, vspace, hdr->e_entry);
    if (IS_ERR(task))
    {
        return PTR_ERR(task);
    }

    hal_enable_interrupts();

    return 0;
}
