#include "Boot.h"

// ToyBoot的入口函数
EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    // 接收中间函数的返回值
    EFI_STATUS Status = EFI_SUCCESS; 
     
    // 如果要打印调试信息，则Logo会因为输出调试信息而错位
    // 所以如果要进行调试，那么就不打印Logo和进度条
    // 日志模块：如果Setup.h文件里定义了LOG，则进行初始化
    #ifdef LOG
    Status = LogOpen(ImageHandle);
    #endif
    
    // 需要传递给KernelEntry()的参数
    BOOT_CONFIG BootConfig; 
    // 设置视频模式，主要是看是否有合适的分辨率
    //VIDEO_CONFIG VideoConfig;
    Status = VideoInit(ImageHandle, &BootConfig.VideoConfig);

    if(EFI_ERROR(Status))
    {
        return Status;
    }

    #ifndef DEBUG
    Status = DrawLogo(ImageHandle);
    #endif

    // 获取Kernel.elf的入口点
    Status = GetElfEntry(ImageHandle, L"\\Kernel.elf", & BootConfig.KernelEntryPoint);

    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = GetFontBmp(ImageHandle, L"\\ASCII.BMP", &BootConfig.AsciiBmp);

    if(EFI_ERROR(Status))
    {
        return Status;
    }

    Status = GetMadt(&BootConfig.MadtAddress);

    if(EFI_ERROR(Status))
    {
        return Status;
    }
 
    #ifdef LOG
    Status = LogClose();
    #endif
   
    Status = JumpToKernel(ImageHandle, &BootConfig);

    return Status;
}

EFI_STATUS GetFontBmp(EFI_HANDLE ImageHandle, CHAR16 *FileName, BMP_CONFIG *BmpConfig)
{
    // 获取字体图片
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to GetFontBmp().\n");
    #endif
    EFI_FILE_PROTOCOL *Ascii;
    Status = GetFileHandle(ImageHandle, FileName, &Ascii);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFontBmp/GetFileHandle().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetFontBmp/GetFileHandle().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetFontBmp/GetFileHandle().\n");
    }else {
        LogWrite("SUCCESS: GetFontBmp/GetFileHandle().\n");
    }
    #endif
    EFI_PHYSICAL_ADDRESS AsciiAddress;
    Status = ReadFile(Ascii, &AsciiAddress);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFontBmp/ReadFile().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetFontBmp/ReadFile().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetFontBmp/ReadFile().\n");
    }else {
        LogWrite("SUCCESS: GetFontBmp/ReadFile().\n");
    }
    #endif
    Status = BmpTransform(AsciiAddress, BmpConfig);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFontBmp/BmpTransform().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetFontBmp/BmpTransform().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetFontBmp/BmpTransform().\n");
    }else {
        LogWrite("SUCCESS: GetFontBmp/BmpTransform().\n");
    }
    #endif
    return Status;
}

EFI_STATUS GetMadt(EFI_PHYSICAL_ADDRESS *MadtAddress)
{
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to GetMadt().\n");
    #endif

    VOID *VendorTable;
    Status = EfiGetSystemConfigurationTable(&gEfiAcpiTableGuid, &VendorTable);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetMadt/EfiGetSystemConfigurationTable().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetMadt/EfiGetSystemConfigurationTable().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetMadt/EfiGetSystemConfigurationTable().\n");
    }else {
        LogWrite("SUCCESS: GetMadt/EfiGetSystemConfigurationTable().\n");
    }
    #endif
    UINTN i = 0;
    RSDP *Rsdp = (RSDP *)VendorTable;
    #ifdef DEBUG
    Print(L"RSDP Signature:");
    
    for(i = 0; i < 7; i++)
    {
        Print(L"%c", Rsdp->Signature[i]);
    }

    Print(L"\nXSDT Address: 0x%08x\n", Rsdp->XsdtAddress);
    #endif
    XSDT *Xsdt = (XSDT *)Rsdp->XsdtAddress;
    MADT *Madt;
    UINTN EntryCount = (Xsdt->Header.Length - sizeof(Xsdt->Header)) / 8;
    
    //Print(L"EntryCount:%d\n", EntryCount);
    for(i = 0; i < EntryCount; i++)
    {
        SDT_HEADER *Header = (SDT_HEADER *)Xsdt->PointerOthers[i];
        if(!AsciiStrnCmp(Header->Signature, "APIC", 4))
        {
            Madt = (MADT *)Header;
            #ifdef DEBUG
            Print(L"Signature:");
            UINTN j;
            for(j = 0; j < 4; j++)
            {
                AsciiPrint("%c", Header->Signature[j]);
            }
            Print(L"\nLapicAddress:0x%x\n", Madt->LapicAddress);
            #endif
        }
        
    }
    
    UINT32 PageCount = (Madt->Header.Length >> 12) + 1;
    //Print(L"PageCount %d, Madt->Header.Lengh %d\n", PageCount, Madt->Header.Length);
    EFI_PHYSICAL_ADDRESS MadtBuffer;
    Status = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, PageCount, &MadtBuffer);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetMadt/gBS->AllocatePages().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetMadt/gBS->AllocatePages().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetMadt/gBS->AllocatePages().\n");
    }else {
        LogWrite("SUCCESS: GetMadt/gBS->AllocatePages().\n");
    }
    #endif

    CopyMem((VOID*)MadtBuffer, (VOID*)Madt, Madt->Header.Length);
    *MadtAddress = MadtBuffer;
 
    #ifdef DEBUG
    Print(L"Contents of MADT:\n");
    UINT8 *Test = (UINT8 *)MadtBuffer;
    for(i = 0; i < Madt->Header.Length; i++)
    {
        if(i == 44)
        {
            Print(L"\nContents of MADT Entries below:\n");
        }
        AsciiPrint("%x ", Test[i]);
    }
    Print(L"\n");
    #endif
    return Status;
}

EFI_STATUS JumpToKernel(EFI_HANDLE ImageHandle, BOOT_CONFIG *BootConfig)
{
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to JumpToKernel().\n");
    #endif
    MEMORY_MAP MemoryMap = {NULL, 1, 0, 0, 0};

    Status = gBS->GetMemoryMap(
                &MemoryMap.MapSize,
                (EFI_MEMORY_DESCRIPTOR*)MemoryMap.Buffer,
                &MemoryMap.MapKey,
                &MemoryMap.DescriptorSize,
                &MemoryMap.DescriptorVersion);
    
    if(Status == EFI_BUFFER_TOO_SMALL)
    {
        #ifdef DEBUG
        Print(L"MemoryMap Buffer is too small. you need at least %d bytes.\n", MemoryMap.MapSize);
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

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetMemory/gBS->GetMemoryMap.\n");
    }else {
        LogWrite("SUCCESS: GetMemory/gBS->GetMemoryMap.\n");
    }
    #endif
    BootConfig->MemoryMap = MemoryMap;
    
    //Status = gBS->ExitBootServices(ImageHandle, MemoryMap.MapKey);
    Print(L"%r\n", Status);
    //while(1);
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/gBS->ExitBootService().\n", Status);
        #endif
        return Status;
    }
    UINT64 (*KernelEntry)(BOOT_CONFIG *BootConfig);
    KernelEntry = (UINT64 (*)(BOOT_CONFIG *BootConfig))BootConfig->KernelEntryPoint;
    UINT64 PassBack = KernelEntry(BootConfig);
    Print(L"Cannot jump to KernelEntry. PassBack=%d.\n", PassBack);
    //Never return here
    return Status;
}