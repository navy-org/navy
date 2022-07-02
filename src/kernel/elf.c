#include <elf/base.h>
#include <string.h>

#include "task.h"

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

    return NULL;
}