#pragma once

#include "abstraction.h"

#define lock$(name) \
	cli(); \
	while (!__sync_bool_compare_and_swap(&name, 0, 1)); \
	__sync_synchronize(); 
#define unlock$(name) \
	__sync_synchronize(); \
	name = 0; \
	sti();