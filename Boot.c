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
    // 日志模块：如果Setup.h文件里定义了LOG，则进行初始化
    #ifdef LOG
    Status = LogOpen(ImageHandle);
    Status = LogError(EFI_NOT_FOUND, "EFI_NOT_FOUND");
    //Status = LogClose();
    #endif
 
    // 设置视频模式，主要是看是否有合适的分辨率
    VIDEO_CONFIG VideoConfig;
    Status = VideoInit(ImageHandle, &VideoConfig);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Cannot set VideoMode correctly.");
    }else {
        LogWrite("Video is good now.\n");
    }
    #endif
    BootConfig.VideoConfig = VideoConfig;
    #ifndef DEBUG
    DrawStep();
    Status = DrawLogo(ImageHandle);
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Cannot drawLogo");
    }else
    {
        LogWrite("Logo is on the screen.\n");
    }
    #endif
    #ifndef DEBUG
    DrawStep();
    #endif

    // 获取Kernel.elf的入口点
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    Status = GetElfEntry(ImageHandle, L"\\Kernel.elf", &KernelEntryPoint);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Cannot GetElfEntry");
    }else
    {
        LogWrite("Kernel entry getted.\n");
    }
    #endif
    UINT64 (*KernelEntry)(BOOT_CONFIG *BootConfig);
    KernelEntry = (UINT64 (*)(BOOT_CONFIG *BootConfig))KernelEntryPoint;
    #ifndef DEBUG
    DrawStep();
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
    
    Status = GetMadt(&BootConfig.MadtAddress);
    Print(L"Madt Address:0x%x", BootConfig.MadtAddress);

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
    #ifdef DEBUG
    if(EFI_ERROR(Status)){
        Print(L"Could not exit boot services : %r.\n",Status);
    }
    #endif
   
    //Status = ByeBootServices(ImageHandle, &BootConfig.MemoryMap);
    //
    UINT64 PassBack = KernelEntry(&BootConfig);
    Print(L"PassBack=%d.\n", PassBack);
    //Never return here
    return Status;
}


EFI_STATUS GetMadt(EFI_PHYSICAL_ADDRESS *MadtAddress)
{
    EFI_STATUS Status = EFI_SUCCESS;
    VOID *VendorTable;
    Status = EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid, &VendorTable);
    if(EFI_ERROR(Status))
    {
        Print(L"EfiGetSystemConfigurationTable error.\n");
        return Status;
    }
    
    RSDP *Rsdp = (RSDP *)VendorTable;

    Print(L"RSDP Signature:");
    UINTN i = 0;
    for(i = 0; i < 7; i++)
    {
        Print(L"%c", Rsdp->Signature[i]);
    }

    Print(L"\nXSDT Address: 0x%08x\n", Rsdp->XsdtAddress);
    XSDT *Xsdt = (XSDT *)Rsdp->XsdtAddress;
    MADT *Madt;
    UINTN EntryCount = (Xsdt->Header.Length - sizeof(Xsdt->Header)) / 8;
    
    Print(L"EntryCount:%d\n", EntryCount);
    for(i = 0; i < EntryCount; i++)
    {
        SDT_HEADER *Header = (SDT_HEADER *)Xsdt->PointerOthers[i];
        if(!AsciiStrnCmp(Header->Signature, "APIC", 4))
        {
            Print(L"Signature:");
            UINTN j;
            for(j = 0; j < 4; j++)
            {
                AsciiPrint("%c", Header->Signature[j]);
            }
            Madt = (MADT *)Header;
            //MadtConfig->MadtAddress = (UINT64)Header;
            Print(L"\n%x %x\n", Header, (UINT64)Header);
            Print(L"\nLapicAddress:0x%x\n", Madt->LapicAddress);
        }
        
    }
    
    UINT32 PageCount = (Madt->Header.Length >> 12) + 1;
    Print(L"PageCount %d, Madt->Header.Lengh %d\n", PageCount, Madt->Header.Length);
    EFI_PHYSICAL_ADDRESS MadtBuffer;
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, PageCount, &MadtBuffer);
    if(EFI_ERROR(Status))
    {
        Print(L"AllocatePages error \r.\n", Status);
        return Status;
    }

    CopyMem((VOID*)MadtBuffer, (VOID*)Madt, Madt->Header.Length);
    *MadtAddress = MadtBuffer;
 
    UINT8 *Test = (UINT8 *)MadtBuffer;
    for(i = 0; i < Madt->Header.Length; i++)
    {
        if(i == 44)
        {
            Print(L"\n");
        }
        AsciiPrint("%c ", Test[i]);
    }
    return Status;
}