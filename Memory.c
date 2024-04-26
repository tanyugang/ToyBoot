#include "Memory.h"

EFI_STATUS GetMemoryMap(MEMORY_MAP *MemoryMap)
{
    EFI_STATUS Status = EFI_SUCCESS;
    //MEMORY_MAP MemoryMap = {1, NULL, 1, 0, 0, 0};

    MemoryMap->Buffer = NULL;
    MemoryMap->MapSize = 1;
    MemoryMap->MapKey = 0;
    MemoryMap->DescriptorSize = 0;
    MemoryMap->DescriptorVersion = 0;

    Status = gBS->GetMemoryMap(
                &MemoryMap->MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap->Buffer,
                &MemoryMap->MapKey,
                &MemoryMap->DescriptorSize,
                &MemoryMap->DescriptorVersion);
    
    if(Status == EFI_BUFFER_TOO_SMALL)
    {
        #ifdef DEBUG
        Print(L"MemoryMap Buffer is too small.\n");
        #endif
    }
    Print(L"MemoryMap Size:%d\n", MemoryMap->MapSize);
    MemoryMap->MapSize = ((MemoryMap->MapSize >> 12) + 1) << 12;
    Print(L"MemoryMap Size:%d\n", MemoryMap->MapSize);
    Status = gBS->AllocatePool(EfiLoaderData, MemoryMap->MapSize, &MemoryMap->Buffer);
    if(EFI_ERROR(Status)){
    Print(L"Failed to allocate memory to get memory map.\n");
    return Status;
    }
    Status = gBS->GetMemoryMap(
                &MemoryMap->MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap->Buffer,
                &MemoryMap->MapKey,
                &MemoryMap->DescriptorSize,
                &MemoryMap->DescriptorVersion);
    return Status;
}
