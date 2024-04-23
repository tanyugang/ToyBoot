#include "Boot.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS;   
    #ifndef DEBUG
    UINT8 Step = 1;
    #endif
    
    VIDEO_CONFIG VideoConfig;

    #ifdef LOG
    Status = LogInitial(ImageHandle);
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Log is good now.\n");
    }
    #endif   

    Status = VideoInit(ImageHandle, &VideoConfig);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Video is good now.\n");
    }
    #endif 

    #ifndef DEBUG
    DrawStep(Step++);
    Status = DrawLogo(ImageHandle);
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Logo is on the screen.\n");
    }
    #endif
    #ifndef DEBUG
    DrawStep(Step++);
    #endif
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    Status = Relocate(ImageHandle, L"\\Kernel.elf", &KernelEntryPoint);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Kernel entry getted.\n");
    }
    #endif
    #ifndef DEBUG
    DrawStep(Step++);
    #endif

    EFI_FILE_PROTOCOL *Ascii;
    Status = GetFileHandle(ImageHandle, L"ASCII.BMP", &Ascii);
     
    EFI_PHYSICAL_ADDRESS AsciiAddress;
    Status = ReadFile(Ascii, &AsciiAddress);

    BMP_CONFIG BmpConfig;
    Status = BmpTransform(AsciiAddress, &BmpConfig);

    BOOT_CONFIG BootConfig;
    BootConfig.VideoConfig = VideoConfig;
    BootConfig.AsciiBmp = BmpConfig;


    
    UINT64 (*KernelEntry)(BOOT_CONFIG *BootConfig);
    KernelEntry = (UINT64 (*)(BOOT_CONFIG *BootConfig))KernelEntryPoint;
    // = {4096, NULL, 4096, 0, 0, 0};
    BootConfig.MemoryMap.BufferSize = 4096;
    BootConfig.MemoryMap.Buffer = NULL;
    BootConfig.MemoryMap.MapSize = 4096 * 4;
    BootConfig.MemoryMap.MapKey = 0;
    BootConfig.MemoryMap.DescriptorSize = 0;
    BootConfig.MemoryMap.DescriptorVersion = 0;
    Status = ByeBootServices(ImageHandle, &BootConfig.MemoryMap);
    UINT64 PassBack = KernelEntry(&BootConfig);
    Print(L"PassBack=%d.\n", PassBack);
    //Never return here
    return Status;
}

EFI_STATUS ByeBootServices(EFI_HANDLE ImageHandle, OUT MEMORY_MAP *MemoryMap)
{
    EFI_STATUS Status = EFI_SUCCESS;
    
    Status = gBS->AllocatePool(EfiLoaderData, MemoryMap->BufferSize, &MemoryMap->Buffer);
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

    Status = gBS->ExitBootServices(ImageHandle, MemoryMap->MapKey);
    if(EFI_ERROR(Status)){
        Print(L"Could not exit boot services : %r.\n",Status);
    }

    return Status;
}

