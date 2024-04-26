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
    VIDEO_CONFIG VideoConfig;
    Status = VideoInit(ImageHandle, &VideoConfig);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/VideoInit().\n");
    }else {
        LogWrite("SUCCESS: Boot/VideoInit().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/VideoInit().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/VideoInit().\n");
    #endif
    #ifndef DEBUG
    DrawStep();
    #endif   
    BootConfig.VideoConfig = VideoConfig;
    #ifndef DEBUG
    Status = DrawLogo(ImageHandle);
    DrawStep();
    #endif

    // 获取Kernel.elf的入口点
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    Status = GetElfEntry(ImageHandle, L"\\Kernel.elf", &KernelEntryPoint);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/GetElfEntry().\n");
    }else {
        LogWrite("SUCCESS: Boot/GetElfEntry().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/GetElfEntry().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/GetElfEntry().\n");
    #endif
    #ifndef DEBUG
    DrawStep();
    #endif
    
    // 
    BMP_CONFIG BmpConfig;
    Status = GetFontBmp(ImageHandle, L"\\ASCII.BMP", &BmpConfig);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/GetFontBmp().\n");
    }else {
        LogWrite("SUCCESS: Boot/GetFontBmp().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/GetFontBmp().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/GetFontBmp().\n");
    #endif
    BootConfig.AsciiBmp = BmpConfig;
    #ifndef DEBUG
    DrawStep();
    #endif
    Status = GetMadt(&BootConfig.MadtAddress);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/GetMadt().\n");
    }else {
        LogWrite("SUCCESS: Boot/GetMadt().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/GetMadt().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/GetMadt().\n");
    #endif
    #ifndef DEBUG
    DrawStep();
    #endif
    MEMORY_MAP MemoryMap;
    Status = GetMemoryMap(&MemoryMap);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/GetMemoryMap().\n");
    }else {
        LogWrite("SUCCESS: Boot/GetMemoryMap().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/GetMemoryMap().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/GetMemoryMap().\n");
    #endif
    BootConfig.MemoryMap = MemoryMap;
    #ifndef DEBUG
    DrawStep();
    #endif

    Status = LogClose();
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/LogClose().\n");
    }else {
        LogWrite("SUCCESS: Boot/LogClose().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/LogClose().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/LogClose().\n");
    #endif
    #ifndef DEBUG
    DrawStep();
    #endif
    Status = gBS->ExitBootServices(ImageHandle, BootConfig.MemoryMap.MapKey);
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to Boot/gBS->ExitBootService().\n");
    }else {
        LogWrite("SUCCESS: Boot/gBS->ExitBootService().\n");
    }
    #endif

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to Boot/gBS->ExitBootService().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Boot/gBS->ExitBootService().\n");
    #endif
    //Status = ByeBootServices(ImageHandle, &BootConfig.MemoryMap);
    UINT64 (*KernelEntry)(BOOT_CONFIG *BootConfig);
    KernelEntry = (UINT64 (*)(BOOT_CONFIG *BootConfig))KernelEntryPoint;
    UINT64 PassBack = KernelEntry(&BootConfig);
    Print(L"Cannot jump to KernelEntry. PassBack=%d.\n", PassBack);
    //Never return here
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
    
    RSDP *Rsdp = (RSDP *)VendorTable;
    #ifdef DEBUG
    Print(L"RSDP Signature:");
    UINTN i = 0;
    for(i = 0; i < 7; i++)
    {
        Print(L"%c", Rsdp->Signature[i]);
    }

    Print(L"\nXSDT Address: 0x%08x\n", Rsdp->XsdtAddress);
    #endif
    XSDT *Xsdt = (XSDT *)Rsdp->XsdtAddress;
    MADT *Madt;
    UINTN EntryCount = (Xsdt->Header.Length - sizeof(Xsdt->Header)) / 8;
    
    Print(L"EntryCount:%d\n", EntryCount);
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
    Print(L"PageCount %d, Madt->Header.Lengh %d\n", PageCount, Madt->Header.Length);
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
    #endif
    return Status;
}