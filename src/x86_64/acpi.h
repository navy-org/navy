#pragma once 

#include <handover/handover.h>
#include <copland/base.h>

typedef struct PACKED
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdtAddress;
} Rsdp;

typedef struct PACKED
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oemtableid[8];
    uint32_t oemrevision;
    uint32_t creatorid;
    uint32_t creatorrevision;
} AcpiSdt;

typedef struct PACKED
{
    AcpiSdt header;
    uint32_t sdtAddr[];
} Rsdt;

typedef Option(AcpiSdt *) AcpiSdtOption;

void acpi_init(Handover const *handover);
AcpiSdtOption acpi_parse(Str tablename);
void acpi_checksum(AcpiSdt *table);