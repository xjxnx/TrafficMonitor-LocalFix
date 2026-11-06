#pragma once
#include <Windows.h>
#include <iphlpapi.h>
#include <cstdlib>

using GetIfTableFunction = DWORD (WINAPI*)(PMIB_IFTABLE, PULONG, BOOL);

// Retry when adapters change between sizing and reading. Publish a replacement
// only after a successful read; failures leave ownership of the old table intact.
inline DWORD RefreshNetworkTable(MIB_IFTABLE*& table, DWORD& capacity,
    GetIfTableFunction read = &GetIfTable)
{
    DWORD required = capacity;
    DWORD result = read(table, &required, FALSE);
    if (result != ERROR_INSUFFICIENT_BUFFER)
        return result;
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        if (required < sizeof(MIB_IFTABLE))
            return ERROR_INVALID_DATA;
        const DWORD allocated = required;
        auto replacement = static_cast<MIB_IFTABLE*>(malloc(allocated));
        if (replacement == nullptr)
            return ERROR_NOT_ENOUGH_MEMORY;
        result = read(replacement, &required, FALSE);
        if (result == NO_ERROR)
        {
            free(table);
            table = replacement;
            capacity = allocated;
            return NO_ERROR;
        }
        free(replacement);
        if (result != ERROR_INSUFFICIENT_BUFFER)
            return result;
        if (required <= allocated)
            return ERROR_INVALID_DATA;
    }
    return ERROR_RETRY;
}
