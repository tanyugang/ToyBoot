#include "Log.h"

#pragma pack(1)
typedef struct MEMORY_MAP
{
    VOID *Buffer;
    UINTN MapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MEMORY_MAP;
#pragma pack()

EFI_STATUS GetMemoryMap(MEMORY_MAP *MemoryMap);