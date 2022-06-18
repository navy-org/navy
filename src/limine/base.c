#include "base.h"
#include "limine.h"

#include <copland/base.h>
#include <handover/handover.h>

static struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

static struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

MAYBE_UNUSED static struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static struct limine_kernel_address_request addr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static void parse_memmap(Handover *self, struct limine_memmap_entry **entries, size_t count)
{
    MmapEntry *m;
    struct limine_memmap_entry *entry;

    self->mmap_count = count;

    log$("=== Memory maps ===");

    for (size_t i = 0; i < count; i++)
    {
        m = &self->mmaps[i];
        entry = entries[i];

        switch(entry->type)
        {
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            {
                m->type = MEMMAP_BOOTLOADER_RECLAIMABLE;
                break;
            }
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            {
                m->type = MEMMAP_KERNEL_AND_MODULES;
                break;
            }
            case LIMINE_MEMMAP_FRAMEBUFFER:
            {
                m->type = MEMMAP_FRAMEBUFFER;
                break;
            }
            case LIMINE_MEMMAP_USABLE:
            {
                m->type = MEMMAP_USABLE;
                break;
            }
            default:
            {
                m->type = MEMMAP_RESERVED;
            }
        }

        m->range = (Range) {
            .base = entry->base,
            .length = entry->length
        };

        log$("{e}: {}", m->range, mmap_type_str[m->type]);
    }
}

MAYBE_UNUSED static void parse_module(Handover *handover, struct limine_file **entries, size_t count)
{
    handover->module_count = count;

    Module *m;
    struct limine_file *entry;

    log$("=== Modules ===");

    for (size_t i = 0; i < count; i++)
    {
        m = &handover->modules[i];
        entry = entries[i];

        m->name = str$(entry->path);
        m->range = (Range) {
            .base = (uintptr_t) entry->address,
            .length = entry->size
        };

        log$("{e}: {}", m->range, m->name);
    }
}


ResultHandover handover_create(void)
{
    Handover result;

    if (memmap_request.response == NULL || memmap_request.response->entry_count == 0)
    {
        return ERR(ResultHandover, str$("Couldn't get Memmaps"));
    }

    if (hhdm_request.response == NULL)
    {
        return ERR(ResultHandover, str$("Couldn't get HHDM"));
    }

    if (addr_request.response == NULL)
    {
        return ERR(ResultHandover, str$("Couldn't get kernel address"));
    }

    if (rsdp_request.response == NULL)
    {
        return ERR(ResultHandover, str$("Couldn't find RSDP address"));
    }

    // if (module_request.response == NULL)
    // {
    //     return ERR(ResultHandover, str$("Couldn't get modules"));
    // }

    result.hhdm_offset = hhdm_request.response->offset;
    result.kernel_vbase = addr_request.response->virtual_base;
    result.kernel_pbase = addr_request.response->physical_base;
    result.rsdp_address = (uintptr_t) rsdp_request.response->address;
    parse_memmap(&result, memmap_request.response->entries, memmap_request.response->entry_count);
    // parse_module(&result, module_request.response->modules, module_request.response->module_count);

    return OK(ResultHandover, result);
}