#include <pmm>

#include "unistd.h"

int getpagesize(void)
{
    return PMM_PAGE_SIZE;
}
