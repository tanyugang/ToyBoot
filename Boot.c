#include "Boot.h"

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status = EFI_SUCCESS; 
    BOOT_CONFIG BootConfig;  
    // 如果要打印调试信息，则Logo会因为输出调试信息而错位
    // 所以如果要进行调试，那么就不打印Logo和进度条
    #ifndef DEBUG
    UINT8 Step = 1;
    #endif
    // 日志模块：如果Setup.h文件里定义了LOG，则进行初始化
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
    
    // 设置视频模式，主要是看是否有合适的分辨率
    VIDEO_CONFIG VideoConfig;
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
    while(1);
    BootConfig.VideoConfig = VideoConfig;
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

    // 获取Kernel.elf的入口点
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    Status = GetElfEntry(ImageHandle, L"\\Kernel.elf", &KernelEntryPoint);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status);
    }else
    {
        LogTip("Kernel entry getted.\n");
    }
    #endif
    UINT64 (*KernelEntry)(BOOT_CONFIG *BootConfig);
    KernelEntry = (UINT64 (*)(BOOT_CONFIG *BootConfig))KernelEntryPoint;
    #ifndef DEBUG
    DrawStep(Step++);
    #endif
    // 获取字体图片
    EFI_FILE_PROTOCOL *Ascii;
    Status = GetFileHandle(ImageHandle, L"ASCII.BMP", &Ascii);
     
    EFI_PHYSICAL_ADDRESS AsciiAddress;
    Status = ReadFile(Ascii, &AsciiAddress);

    BMP_CONFIG BmpConfig;
    Status = BmpTransform(AsciiAddress, &BmpConfig);
    // 
    
    BootConfig.AsciiBmp = BmpConfig;
    
   
    MEMORY_MAP MemoryMap = {4096, NULL, 4096, 0, 0, 0};

    Status = gBS->AllocatePool(EfiLoaderData, MemoryMap.BufferSize, &MemoryMap.Buffer);
    
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
    
    if(Status == EFI_BUFFER_TOO_SMALL)
    {
        #ifdef DEBUG
        Print(L"MemoryMap Buffer is too small.\n");
        #endif
    }
    Print(L"MemoryMap Size:%x\n");
    MemoryMap.MapSize = MemoryMap.MapSize + 4096;
    Status = gBS->GetMemoryMap(
                &MemoryMap.MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap.Buffer,
                &MemoryMap.MapKey,
                &MemoryMap.DescriptorSize,
                &MemoryMap.DescriptorVersion);
    BootConfig.MemoryMap = MemoryMap;
    LogClose();
    Status = gBS->ExitBootServices(ImageHandle, MemoryMap.MapKey);
    if(EFI_ERROR(Status)){
        Print(L"Could not exit boot services : %r.\n",Status);
    }

   
    //Status = ByeBootServices(ImageHandle, &BootConfig.MemoryMap);
    //
    UINT64 PassBack = KernelEntry(&BootConfig);
    Print(L"PassBack=%d.\n", PassBack);
    //Never return here
    return Status;
}
