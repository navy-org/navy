#include <dbg/log.h>
#include <hal.h>
#include <handover.h>
#include <stddef.h>
#include <string.h>

#include "handover/builder.h"
#include "handover/handover.h"
#include "limine.h"

/* --- Handover ------------------------------------------------------------- */

static uint8_t handover_buffer[kib$(16)] = {0};
static HandoverBuilder builder;
static bool is_handover_init = false;

/* --- Limine requests ------------------------------------------------------ */

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

void handover_parse_mmap(HandoverBuilder *self)
{
    if (memmap_req.response == NULL)
    {
        error$("Couldn't retrieve memory map from Limine");
        hal_panic();
    }

    if (kernel_addr_req.response == NULL)
    {
        error$("Couldn't retrieve kernel address from Limine");
        hal_panic();
    }

    log$("Retrieved memory map from Limine");

    log$("======================================================");
    log$("    TYPE     |       BASE         |       LIMIT       ");
    log$("======================================================");

    for (size_t i = 0; i < memmap_req.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_req.response->entries[i];
        HandoverRecord record = {0};

        switch (entry->type)
        {
            case LIMINE_MEMMAP_USABLE:
            {
                log$("FREE         | %p | %p", entry->base, entry->base + entry->length);
                record.tag = HANDOVER_FREE;
                break;
            }

            case LIMINE_MEMMAP_ACPI_NVS:
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_BAD_MEMORY:
            {
                log$("RESERVED     | %p | %p", entry->base, entry->base + entry->length);
                record.tag = HANDOVER_RESERVED;
                break;
            }

            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            {
                log$("RECLAIMABLE  | %p | %p", entry->base, entry->base + entry->length);
                record.tag = HANDOVER_LOADER;
                break;
            }

            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
            {
                log$("MODULE      | %p | %p", entry->base, entry->base + entry->length);
                record.tag = HANDOVER_KERNEL;
                break;
            }

            case LIMINE_MEMMAP_FRAMEBUFFER:
            {
                log$("FRAMEBUFFER | %p | %p", entry->base, entry->base + entry->length);
                record.tag = HANDOVER_FB;
                break;
            }

            default:
            {
                error$("Unknown memory map entry type %d", entry->type);
                hal_panic();
            }
        }

        record.start = entry->base;
        record.size = entry->length;

        handover_builder_append(self, record);
    }

    log$("=====================================================");
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

void handover_parse_module(HandoverBuilder *self)
{
    if (module_request.response == NULL)
    {
        error$("Couldn't retrieve list of modules from Limine");
        hal_panic();
    }

    HandoverRecord rec = {0};

    for (size_t i = 0; i < module_request.response->module_count; i++)
    {
        size_t str_offset = handover_builder_append_str(self, module_request.response->modules[i]->path);
        rec = (HandoverRecord){
            .tag = HANDOVER_FILE,
            .flags = 0,
            .start = (uintptr_t)module_request.response->modules[i]->address,
            .size = module_request.response->modules[i]->size,
            .file = {
                .name = str_offset,
                .meta = 0,
            },
        };

        handover_builder_append(self, rec);
    }
}

HandoverPayload *handover(void)
{
    if (!is_handover_init)
    {
        handover_builder_init(&builder, handover_buffer, kib$(16));
        handover_parse_module(&builder);
        handover_parse_mmap(&builder);

        is_handover_init = true;
    }

    return builder.payload;
}