#include "Motion.h"
#include "Elf.h"

#pragma pack(1)
typedef struct
{
    UINTN BufferSize;
    VOID *Buffer;
    UINTN MapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MEMORY_MAP;

typedef struct
{
    VIDEO_CONFIG VideoConfig;
    MEMORY_MAP   MemoryMap;
    BMP_CONFIG AsciiBmp;
} BOOT_CONFIG; // @Boot.h

typedef struct
{
    UINT16 LimitLow;
    UINT16 BaseLow;
    UINT8  BaseMiddle;
    UINT8  SysFlag;
    UINT8  LimitHigh;
    UINT8  BaseHigh;
} GDT_SELECTOR;

EFI_STATUS ByeBootServices(
    IN EFI_HANDLE ImageHandle,
    OUT MEMORY_MAP *MemoryMap
);
#pragma pack()