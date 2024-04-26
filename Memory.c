#include "Memory.h"

EFI_STATUS GetMemoryMap(EFI_HANDLE ImageHandle, MEMORY_MAP *MemoryMapOut)
{
    EFI_STATUS Status = EFI_SUCCESS;
    MEMORY_MAP MemoryMap = {NULL, 1, 0, 0, 0};
 
    Print(L"Start to GetMemoryMap().\n");
    Status = gBS->GetMemoryMap(
                &MemoryMap.MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap.Buffer,
                &MemoryMap.MapKey,
                &MemoryMap.DescriptorSize,
                &MemoryMap.DescriptorVersion);
    
    if(Status == EFI_BUFFER_TOO_SMALL)
    {
        #ifdef DEBUG
        Print(L"MemoryMap Buffer is too small.\n");
        #endif
    }
    Print(L"MemoryMap Size:%d\n", MemoryMap.MapSize);
    MemoryMap.MapSize = ((MemoryMap.MapSize >> 12) + 1) << 12;
    Print(L"MemoryMap Size:%d\n", MemoryMap.MapSize);
    Status = gBS->AllocatePool(EfiLoaderData, MemoryMap.MapSize, &MemoryMap.Buffer);
    if(EFI_ERROR(Status)){
    Print(L"Failed to allocate memory to get memory map.\n");
    return Status;
    }
    Status = gBS->GetMemoryMap(
                &MemoryMap.MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap.Buffer,
                &MemoryMap.MapKey,
                &MemoryMap.DescriptorSize,
                &MemoryMap.DescriptorVersion);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetMemory/gBS->GetMemoryMap.\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetMemory/gBS->GetMemoryMap.\n");
    #endif

    MemoryMapOut->Buffer = MemoryMap.Buffer;
    MemoryMapOut->MapSize = MemoryMap.MapSize;
    MemoryMapOut->MapKey = MemoryMap.MapKey;
    MemoryMapOut->DescriptorSize = MemoryMap.DescriptorSize;
    MemoryMapOut->DescriptorVersion = MemoryMap.DescriptorVersion;

    return Status;
}
