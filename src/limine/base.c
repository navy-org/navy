#include "base.h"
#include "copland/debug.h"
#include "limine.h"

#include <x86_64/base.h>
#include <copland/base.h>
#include <handover/handover.h>

#include <kernel/pmm.h>

extern struct limine_memmap_request memmap_request;
extern struct limine_hhdm_request hhdm_request;
extern struct limine_kernel_address_request addr_request;
extern struct limine_rsdp_request rsdp_request;
extern struct limine_smp_request smp_request;
extern struct limine_module_request module_request;

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
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            {
                m->type = MEMMAP_ACPI_RECLAIMABLE;
                break;
            }
            case LIMINE_MEMMAP_ACPI_NVS:
            {
                m->type = MEMMAP_ACPI_NVS;
                break;
            }
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
            case LIMINE_MEMMAP_RESERVED:
            {
                m->type = MEMMAP_RESERVED;
                break;
            }
            default:
            {
                panic$("Unknown memmap type {}", entry->type);
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

static void smp_idle(struct limine_smp_info *cpu)
{
    cpu->goto_address(cpu);
}

void handover_goto_core(int id, CoreGoto func)
{
    struct limine_smp_response *response = smp_request.response;

    if (response == NULL)
    {
        return;
    }

    response->cpus[id]->goto_address = (void(*)(struct limine_smp_info *)) func;
}

static void parse_smp(Handover *handover, struct limine_smp_response *response)
{
    log$("Booting {} cores", response->cpu_count);
    SmpEntry entry = {
        .core_count = response->cpu_count,
        .cpus = (void **)(response->cpus),
    };

    handover->smp = entry;

    for (size_t i = 0; i < response->cpu_count; i++)
    {
        response->cpus[i]->goto_address = smp_idle;
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

    if (smp_request.response == NULL)
    {
        return ERR(ResultHandover, str$("Couldn't get SMP info"));
    }

    if (module_request.response == NULL)
    {
        return ERR(ResultHandover, str$("Couldn't get modules"));
    }


    parse_smp(&result, smp_request.response);
    result.hhdm_offset = hhdm_request.response->offset;
    result.kernel_vbase = addr_request.response->virtual_base;
    result.kernel_pbase = addr_request.response->physical_base;
    result.rsdp_address = (uintptr_t) rsdp_request.response->address;
    parse_memmap(&result, memmap_request.response->entries, memmap_request.response->entry_count);
    parse_module(&result, module_request.response->modules, module_request.response->module_count);

    return OK(ResultHandover, result);
}

