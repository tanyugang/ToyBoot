#include "Elf.h"

EFI_STATUS GetElfEntry(
    IN EFI_HANDLE ImageHandle,
    IN CHAR16 *FileName,
    OUT EFI_PHYSICAL_ADDRESS *KernelEntry)
{
    EFI_STATUS Status = EFI_SUCCESS;

    EFI_FILE_PROTOCOL *Kernel;
    Status = GetFileHandle(ImageHandle, FileName, &Kernel);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetElfEntry/GetFileHandle().\n", Status);
        #endif
        return Status;
    }

    EFI_PHYSICAL_ADDRESS KernelBuffer;
    Status = ReadFile(Kernel, &KernelBuffer);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetElfEntry/ReadFile().\n", Status);
        #endif
        return Status;
    }  

    Status = CheckELF(KernelBuffer);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetElfEntry/CheckELF().\n", Status);
        #endif
        return Status;
    }

    Status = LoadSegments(KernelBuffer, KernelEntry);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetElfEntry/LoadSegments().\n", Status);
        #endif
        return Status;
    }

    return Status;
}

EFI_STATUS CheckELF(
    IN EFI_PHYSICAL_ADDRESS KernelBuffer
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINT32 Magic = GetValue(KernelBuffer, 0x00, 4);
    if(Magic != 0x464c457F)
    {
        #ifdef DEBUG
        Print(L"ERROR: It is not an Elf file.\n");
        #endif

        Status = NOT_ELF_FILE;
        return Status;
    }

    UINT8 Format = GetValue(KernelBuffer, 0x04, 1);
    if (Format != ELF_64)
    {
        #ifdef DEBUG
        Print(L"ERROR: It is not a 64 bits Elf file.\n");
        #endif
        Status = NOT_64_BIT;
        return Status;
    }
    
    return Status;
}


EFI_STATUS LoadSegments(
    IN EFI_PHYSICAL_ADDRESS KernelBufferBase,
    OUT EFI_PHYSICAL_ADDRESS *KernelEntry
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    ELF_HEADER_64 *ElfHeader = (ELF_HEADER_64 *)KernelBufferBase;
    PROGRAM_HEADER_64 *PHeader = (PROGRAM_HEADER_64 *)(KernelBufferBase + ElfHeader->Phoff);
    
    EFI_PHYSICAL_ADDRESS LowAddr = 0xFFFFFFFFFFFFFFFF;
    EFI_PHYSICAL_ADDRESS HighAddr = 0;

    for (UINTN i = 0; i < ElfHeader->PHeadCount; i++)
    {
        if (PHeader[i].Type == PT_LOAD)
        {
            if (LowAddr > PHeader[i].PAddress)
            {
                LowAddr = PHeader[i].PAddress;
            }
            if (HighAddr < (PHeader[i].PAddress + PHeader[i].SizeInMemory))
            {
                HighAddr = PHeader[i].PAddress + PHeader[i].SizeInMemory;
            }
        }
    }

    UINTN PageCount = ((HighAddr - LowAddr) >> 12) + 1;
    EFI_PHYSICAL_ADDRESS KernelRelocateBase;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderCode,
        PageCount,
        &KernelRelocateBase);
   
   
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: Failed to LoadSegments/gBS->AllocatePages().\n");
        #endif
        return Status;
    }

    UINT64 RelocateOffset = KernelRelocateBase - LowAddr;
    UINT64 *ZeroStart = (UINT64 *)KernelRelocateBase;
    for(UINTN i = 0; i < (PageCount << 9); i++)
    {
        *ZeroStart = 0x000000000000;
        ZeroStart++;
    }

    for (UINTN i = 0; i < ElfHeader->PHeadCount; i++)
    {
        
        if (PHeader[i].Type == PT_LOAD)
        {    
            UINT8 *SourceStart = (UINT8 *)KernelBufferBase + PHeader[i].Offset;
            UINT8 *DestStart = (UINT8 *)PHeader[i].VAddress + RelocateOffset;
            
            for(UINTN j = 0; j < PHeader[i].SizeInFile; j++)
            {
                *DestStart = *SourceStart;                            
                SourceStart++;
                DestStart++;               
            }
        }        
    }
    *KernelEntry = ElfHeader->Entry + RelocateOffset;

    return Status;
}


EFI_STATUS GetElfInfo(
    IN EFI_PHYSICAL_ADDRESS KernelAddress,
    IN OUT EFI_PHYSICAL_ADDRESS *KernelEntry)
{
    EFI_STATUS Status = EFI_SUCCESS;
    ELF_HEADER_64 *ElfHeader = (ELF_HEADER_64 *)KernelAddress;
    PROGRAM_HEADER_64 *PHeader = (PROGRAM_HEADER_64 *)(KernelAddress + ElfHeader->Phoff);
    
    EFI_PHYSICAL_ADDRESS LowAddr = 0xFFFFFFFFFFFFFFFF;
    EFI_PHYSICAL_ADDRESS HighAddr = 0;

    Print(L"KernelBase = %x.\n", KernelAddress);
    for (UINTN i = 0; i < ElfHeader->PHeadCount; i++)
    {
        if (PHeader[i].Type == PT_LOAD)
        {
            if (LowAddr > PHeader[i].PAddress)
            {
                LowAddr = PHeader[i].PAddress;
            }
            if (HighAddr < PHeader[i].PAddress + PHeader[i].SizeInMemory)
            {
                HighAddr = PHeader[i].PAddress + PHeader[i].Offset;
            }
        }
        //PHeader++;
    }

    Print(L"LowAddr:%016llx, HighAddr:%016llx.\n", LowAddr, HighAddr);

    UINTN PageCount = (HighAddr - LowAddr + 4095) / 4096;
    EFI_PHYSICAL_ADDRESS KernelRelocateBase;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderCode,
        PageCount,
        &KernelRelocateBase);
   
    if(EFI_ERROR(Status))
    {
        Print(L"Allocate pages for kernelrelocate error.\n");
        return Status;
    }
    Print(L"KernelRelocateBase = %016llx.\n", KernelRelocateBase);
    UINTN RelocateOffset = KernelRelocateBase - LowAddr;

    Print(L"RelocateOffset:%016llx.\n", RelocateOffset);
    for (UINTN i = 0; i < ElfHeader->PHeadCount; i++)
    {
        if (PHeader[i].Type == PT_LOAD)
        {
            CopyMem(
                (VOID *)(PHeader[i].PAddress + RelocateOffset), (VOID *)(KernelAddress + PHeader[i].Offset), PHeader[i].SizeInFile
            );
            Print(L"Seg %d New Address = %016llx.\n", i, PHeader[i].PAddress + RelocateOffset);
            if(PHeader[i].SizeInMemory > PHeader[i].SizeInFile)
            {
                SetMem(
                    (VOID *)(KernelRelocateBase + PHeader[i].SizeInFile),
                    PHeader[i].SizeInMemory - PHeader[i].SizeInFile,
                    0
                );
            }
        }
        
    }
    *KernelEntry = ElfHeader->Entry + RelocateOffset;
    Print(L"Relocated KernleEntry = %016llx.\n", *KernelEntry);

    return Status;
}
