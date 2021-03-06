#pragma once

#include <copland/base.h>
#include <stdbool.h>

typedef enum 
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
    COM5 = 0x5F8,
    COM6 = 0x4F8,
    COM7 = 0x5E8,
    COM8 = 0x4E8,
} ComPort;

typedef struct 
{
    Writer funcs;
    ComPort port;
} Com;

void com_putc(Writer *self, char c);
Com com_init(ComPort port);
void com_enable_lock(void);