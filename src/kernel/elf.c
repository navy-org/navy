#include <elf/base.h>
#include <string.h>

#include "abstraction.h"
#include "task.h"
#include "pmm.h"

Task *elf_load(Module *m, MAYBE_UNUSED TaskArgs args)
{
    struct elf64_hdr *hdr = (struct elf64_hdr *)  m->range.base;

    if (strncmp((char const *) hdr->e_ident, ELFMAG, 4) != 0)
    {
        panic$("{} is not a valid ELF file", m->name);
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        panic$("{} is not a valid 64 bits ELF", m->name);
    }

    log$("Loading {} {}", m->name, m->range);
    Pml *space = UNWRAP_OR_PANIC(vmm_create_space(), "Couldn't allocate address space");

     for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf64_Phdr *phdr = (Elf64_Phdr *)((uintptr_t) m->range.base + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            Range addr = UNWRAP_OR_PANIC(pmm_alloc(align_up$(phdr->p_memsz, PAGE_SIZE)), "Out of memory");
            vmm_map_range(space, (Range) {
                .base = align_down$(phdr->p_vaddr, PAGE_SIZE),
                .length = align_up$(addr.length, PAGE_SIZE),
            }, addr, true);

            memcpy((void *) (addr.base + get_hhdm_offset()), (void *) (m->range.base + phdr->p_offset), phdr->p_filesz);
            memcpy((void *) addr.base + get_hhdm_offset() + phdr->p_filesz, (void *) (m->range.base + phdr->p_offset + phdr->p_filesz), phdr->p_memsz - phdr->p_filesz);
        }

        phdr = (Elf64_Phdr *) (m->range.base + hdr->e_phoff + (i * hdr->e_phentsize));
    }

    Task *task = task_create(m->name, space);
    context_create(&task->ctx, hdr->e_entry, args);

    return task;
}