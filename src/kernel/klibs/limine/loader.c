#include <dbg/log.h>
#include <hal.h>
#include <loader.h>
#include <stddef.h>
#include <string.h>

#include "limine.h"

static Mmap mmap = {0};

/* --- Limine requests ----------------------------------------------------- */

static volatile struct limine_memmap_request memmap_req = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL,
};

static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = NULL,
};

static volatile struct limine_kernel_address_request kernel_addr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = NULL,
};

static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = NULL,
};

volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .response = 0,
    .revision = 0,
};

/* --- Loader functions ---------------------------------------------------- */

Mmap loader_get_mmap(void)
{
    if (mmap.len > 0)
    {
        return mmap;
    }

    if (memmap_req.response == NULL)
    {
        error$("Couldn't retrieve memory map from Limine");
        hal_panic();
    }

    log$("Retrieved memory map from Limine");

    log$("=====================================================");
    log$("    TYPE    |       BASE         |       LIMIT       ");
    log$("=====================================================");

    size_t i;
    for (i = 0; i < memmap_req.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_req.response->entries[i];
        MmapEntry *mmap_entry = &mmap.entries[i];

        switch (entry->type)
        {
            case LIMINE_MEMMAP_USABLE:
            {
                log$("FREE        | %p | %p", entry->base, entry->base + entry->length);
                mmap_entry->type = LOADER_FREE;
                break;
            }

            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_BAD_MEMORY:
            {
                log$("RESERVED    | %p | %p", entry->base, entry->base + entry->length);
                mmap_entry->type = LOADER_RESERVED;
                break;
            }

            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            {
                log$("RECLAIMABLE | %p | %p", entry->base, entry->base + entry->length);
                mmap_entry->type = LOADER_RECLAIMABLE;
                break;
            }

            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            {
                log$("MODULE      | %p | %p", entry->base, entry->base + entry->length);
                mmap_entry->type = LOADER_KERNEL;
                break;
            }

            case LIMINE_MEMMAP_FRAMEBUFFER:
            {
                log$("FRAMEBUFFER| %p | %p", entry->base, entry->base + entry->length);
                mmap_entry->type = LOADER_FB;
                break;
            }

            default:
            {
                error$("Unknown memory map entry type %d", entry->type);
                hal_panic();
            }
        }

        mmap_entry->base = entry->base;
        mmap_entry->len = entry->length;
    }

    log$("=====================================================");

    mmap.len = i;
    return mmap;
}

uintptr_t hal_mmap_l2h(uintptr_t addr)
{
    if (hhdm_req.response == NULL)
    {
        error$("Couldn't convert address from Limine");
        hal_panic();
    }

    return addr + hhdm_req.response->offset;
}

uintptr_t hal_mmap_h2l(uintptr_t addr)
{
    if (hhdm_req.response == NULL)
    {
        error$("Couldn't convert address from Limine");
        hal_panic();
    }

    return addr - hhdm_req.response->offset;
}

KernelMmap loader_get_kernel_mmap(void)
{
    if (kernel_addr_req.response == NULL)
    {
        error$("Couldn't retrieve kernel address from Limine");
        hal_panic();
    }

    KernelMmap kernel_mmap = {
        .phys = kernel_addr_req.response->physical_base,
        .virt = kernel_addr_req.response->virtual_base,
    };

    return kernel_mmap;
}

Rsdp *hal_acpi_rsdp(void)
{
    if (rsdp_req.response == NULL)
    {
        error$("Couldn't retrieve RSDP from Limine");
        hal_panic();
    }

    return (Rsdp *)rsdp_req.response->address;
}

Module loader_get_module(char const *path)
{
    if (module_request.response == NULL)
    {
        return (Module){0};
    }

    for (size_t i = 0; i < module_request.response->module_count; i++)
    {
        if (memcmp(path, module_request.response->modules[i]->path, strlen(path)) == 0)
        {
            Module mod = {
                .base = (uintptr_t)module_request.response->modules[i]->address,
                .len = module_request.response->modules[i]->size,
            };

            memcpy(mod.name, module_request.response->modules[i]->path, strlen(path));

            return mod;
        }
    }

    return (Module){0};
}