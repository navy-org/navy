#include <hal>
#include <logger>
#include <string.h>

#include "acpi.h"

static bool is_xsdt = false;
static Sdt *sdt;

void acpi_init(void)
{
    Rsdp *rsdp = hal_acpi_rsdp();

    if (rsdp->revision >= 2 && rsdp->xsdt_address != 0)
    {
        is_xsdt = true;
        log$("XSDT is supported");
        sdt = (Sdt *)hal_mmap_l2h(rsdp->xsdt_address);
    }
    else
    {
        log$("XSDT is not supported, defaulting to RSDT");
        sdt = (Sdt *)hal_mmap_l2h(rsdp->rsdt_address);
    }
}

static int acpi_checksum(SdtHeader *table)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < table->length; i++)
    {
        sum += ((char *)table)[i];
    }

    return sum == 0;
}

Res acpi_parse_sdt(char tablename[static 1])
{
    size_t entry_count = 0;
    SdtHeader *tmp;

    if (is_xsdt)
    {
        entry_count = sdt->xsdt.header.length - sizeof(sdt->xsdt.header) / 8;
    }
    else
    {
        entry_count = sdt->rsdt.header.length - sizeof(sdt->rsdt.header) / 4;
    }

    for (size_t i = 0; i < entry_count; i++)
    {
        if (is_xsdt)
        {
            tmp = (SdtHeader *)hal_mmap_l2h(sdt->xsdt.entry[i]);
        }
        else
        {
            tmp = (SdtHeader *)hal_mmap_l2h(sdt->rsdt.entry[i]);
        }

        if (memcmp(tmp->signature, tablename, 4) == 0 && acpi_checksum(tmp))
        {
            return uok$(tmp);
        }
    }

    return err$(RES_NOENT);
}
