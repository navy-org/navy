#pragma once

#ifndef __ck_host__
#    error "This file is for host only"
#endif

#include <stdlib.h>
#include <traits>

Alloc *default_alloc(void);