#pragma once

#include <hal>
#include <port>
#include <result>
#include <tinyvmem>

typedef struct
{
    const char *name;
    size_t pid;
    Vmem vmem;
    HalPage *space;
    HalContext *ctx;
    IpcPortList ports;
} Task;

Res task_new(char const *name, Res address_space, Res ip);