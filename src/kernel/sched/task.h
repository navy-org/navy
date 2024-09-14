#pragma once

#include <hal>
#include <navy>
#include <port>
#include <result>
#include <tinyvmem>

typedef struct
{
    const char *name;
    pid_t pid;
    Vmem vmem;
    HalPage *space;
    HalContext *ctx;
    IpcPortList ports;
} Task;

Res task_new(char const *name, Res address_space, Res ip);