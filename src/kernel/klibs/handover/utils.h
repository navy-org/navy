#pragma once

#ifndef HANDOVER_UTILS_H_INCLUDED
#    define HANDOVER_UTILS_H_INCLUDED

#    include <stddef.h>
#    include <string.h>

#    include "handover.h"

static inline size_t handover_add_string(HandoverPayload *handover, const char *str)
{
    size_t len = strlen(str) + 1;
    size_t offset = handover->size - len;
    memset((void *)((uintptr_t)handover + offset), 0, len);
    memcpy((void *)((uintptr_t)handover + offset), str, len);
    handover->size -= len;
    return offset;
}

static inline HandoverRecord handover_file_find(HandoverPayload *handover, char const *name)
{
    for (size_t i = 0; i < handover->count; i++)
    {
        HandoverRecord record = handover->records[i];

        if (record.tag == HANDOVER_FILE)
        {
            char *filename = (char *)handover + record.file.name;

            if (memcmp(filename, name, strlen(filename)) == 0)
            {
                return record;
            }
        }
    }

    return (HandoverRecord){0};
}

#    define handover_foreach_record(h, r)                        \
        if ((h)->count > 0)                                      \
            for (size_t i = 0;                                   \
                 i < (h)->count && (((r) = (h)->records[i]), 1); \
                 ++i)

#endif