#pragma once 

#include <stdint.h>
#include <copland/macro.h>

typedef enum 
{
    GDT_NULL,
    GDT_KERNEL_CODE ,
    GDT_KERNEL_DATA,
    GDT_USER_DATA,
    GDT_USER_CODE,
    GDT_LENGTH
} GdtEnum;

typedef enum
{
    GDT_KERNEL = 0b00000000,
    GDT_USER = 0b01100000,
    GDT_CODE = 0b00011000,
    GDT_DATA = 0b00010000,
} GdtBitField;


typedef union
{
    struct PACKED
    {
        uint16_t base_low;
        uint16_t limit_low;
        uint8_t base_mid;
        uint8_t access;
        uint8_t limit_high : 4;
        uint8_t flags : 4;
        uint8_t base_high;
    };

   uint64_t raw;
} GdtEntry;

typedef struct PACKED 
{
    uint16_t limit;
    uint64_t base;
} GdtPointer;

typedef struct PACKED
{
    uint32_t reserved;
    uint64_t rsp[3];
    uint64_t reserved2;
    uint64_t ist[7];
    unsigned __int128 reserved3 : 80;
    uint16_t iopb;
} Tss;

typedef struct PACKED 
{
    uint16_t length;
    uint16_t base_low16;
    uint8_t base_mid8;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base_high8;
    uint32_t base_upper32;
    uint32_t reserved;
} TssEntry;

typedef struct PACKED
{
    GdtEntry entries[GDT_LENGTH];
    TssEntry tss;
} Gdt;

void gdt_init(void);
void gdt_load_tss(Tss *tss);

extern void gdt_flush(uintptr_t);
extern void tss_flush(void);