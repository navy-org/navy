#pragma once

#include <stdint.h>

typedef struct [[gnu::packed]]
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} Rsdp;

typedef struct [[gnu::packed]]
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemid[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} SdtHeader;

typedef union
{
    SdtHeader *header;
    struct [[gnu::packed]] RSDT
    {
        SdtHeader header;
        uint32_t entry[];
    } rsdt;
    struct [[gnu::packed]] XSDT
    {
        SdtHeader header;
        uint64_t entry[];
    } xsdt;
} Sdt;

void acpi_init(void);
SdtHeader *acpi_parse_sdt(char tablename[static 1]);
