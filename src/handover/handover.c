#include "handover.h"


Module handover_find_module(Handover const *handover, Str name)
{
    for (size_t i = 0; i < handover->module_count; i++)
    {
        if (str_eq(handover->modules[i].name, name))
        {
            return handover->modules[i];
        }
    }

    panic$("{} was not found", name);
    __builtin_unreachable();
}