#include "acpi.h"

static Rsdt *rsdt;
static uint64_t hhdm_offset;

void acpi_init(const Handover *handover)
{
    hhdm_offset = handover->hhdm_offset;
    Rsdp *rsdp = (Rsdp *) (handover->rsdp_address);
    rsdt = (Rsdt *) (rsdp->rsdtAddress + hhdm_offset);
}

void acpi_checksum(AcpiSdt *table)
{
    unsigned char sum = 0;

    for (size_t i = 0; i < table->length; i++)
    {
        sum += ((char *) table)[i];
    }

    if (sum != 0)
    {
        panic$("Checksum for {} FAILED !", table->signature);
    }
}

AcpiSdtOption acpi_parse(Str tablename)
{
    size_t length = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    for (size_t i = 0; i < length; i++)
    {
        AcpiSdt *sdt = (AcpiSdt *) (rsdt->sdtAddr[i] + hhdm_offset);
        if (str_eq(tablename, str_n$(4, sdt->signature)))
        {
            return SOME(AcpiSdtOption, sdt);
        }
    }

    return NONE(AcpiSdtOption);
}