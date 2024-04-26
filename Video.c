#include "Video.h"

EFI_STATUS GetGopHandle(
    IN EFI_HANDLE ImageHandle,
    OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to GetGopHandle().\n");
    #endif
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer;
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetGopHandle/gBS->LocateHanleBuffer() GraphicsOutputProtocol.\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetGopHandle/gBS->LocateHanleBuffer(), %d handle(s) support GraphicsOutputProtocol.\n", HandleCount);
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "GetGopHandle/gBS->LocateHanleBuffer().\n");
    }else {
        LogWrite("SUCCESS: GetGopHandle/gBS->LocateHanleBuffer().\n");
    }
    #endif
    Status = gBS->OpenProtocol(
        HandleBuffer[0],
        &gEfiGraphicsOutputProtocolGuid,
        (VOID **)Gop,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetGopHandle/gBS->OpenProtocol() GraphicsOutputProtocol.\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: GetGopHandle/gBS->OpenProtocol().\n"); 
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "GetGopHandle/gBS->OpenProtocol() GraphicsOutputProtocol.\n");
    }else {
        LogWrite("SUCCESS: GetGopHandle/gBS->OpenProtocol().\n");
    }
    #endif
    return Status;
}

EFI_STATUS SetVideoMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to SetVideoMode().\n");
    #endif
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo;
    UINTN ModeInfoSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    UINTN H = 0;
    UINTN V = 0;
    UINTN ModeIndex = 0;

    for(UINTN i = 0; i < Gop->Mode->MaxMode; i++)
    {
        Status = Gop->QueryMode(Gop, i, &ModeInfoSize, &ModeInfo);
        H = ModeInfo->HorizontalResolution;
        V = ModeInfo->VerticalResolution;
        if(((H == 1024) && (V == 768)) || ((H == 1440) && (V == 900)))
        {    
            ModeIndex = i;            
        }
    }


    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to SetVideoMode/Gop->QueryMode().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: SetVideoMode/Gop->QueryMode(), the best Mode is Mode:%d.\n", ModeIndex);
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to SetVideoMode/Gop->QueryMode().\n");
    }else {
        LogWrite("SUCCESS: SetVideoMode/Gop->QueryMode().\n");
    }
    #endif

    Status = Gop->SetMode(Gop, ModeIndex);  

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to SetVideoMode/Gop->SetMode().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: SetVideoMode/Gop->SetMode() to Index:%d.\n", ModeIndex);
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to SetVideoMode/Gop->SetMode().\n");
    }else {
        LogWrite("SUCCESS: SetVideoMode/Gop->SetMode().\n");
    }
    #endif
    return Status;
}


EFI_STATUS BmpTransform(
    IN EFI_PHYSICAL_ADDRESS BmpBase,
    OUT BMP_CONFIG *BmpConfig
)    
{  
    EFI_STATUS Status = EFI_SUCCESS;
    #ifdef LOG
    Status = LogWrite("Start to BmpTransform().\n");
    #endif
    BmpConfig->Size = GetValue(BmpBase, 0x02, 4);
    BmpConfig->PageSize = (BmpConfig->Size >> 12) + 1;
    BmpConfig->Offset = GetValue(BmpBase, 0x0A, 4);
    
    BmpConfig->Width = GetValue(BmpBase, 0x12, 4);
    BmpConfig->Height = GetValue(BmpBase, 0x16, 4);
    
    EFI_PHYSICAL_ADDRESS PixelStart;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        BmpConfig->PageSize,
        &PixelStart
    );

    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to BmpTransform/gBS->AllocatePages().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: BmpTransform/gBS->AllocatePages().\n");
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to BmpTransform/gBS->AllocatePages().\n");
    }else {
        LogWrite("SUCCESS: BmpTransform/gBS->AllocatePages().\n");
    }
    #endif
    
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelFromFile = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)(BmpBase 
                                                    + BmpConfig->Offset 
                                                    + BmpConfig->Width * BmpConfig->Height * 4);
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelToBuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)PixelStart;
    for(UINTN i = 0; i < BmpConfig->Height; i++)
    {
        PixelFromFile -= BmpConfig->Width;
        for(UINTN j = 0; j < BmpConfig->Width; j++)
        {
            *PixelToBuffer = *PixelFromFile;
            PixelToBuffer++;
            PixelFromFile++;
        }
        PixelFromFile -= BmpConfig->Width;
    }

    BmpConfig->PixelStart = PixelStart;

    return Status;
}

EFI_STATUS DrawBmp(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
    IN BMP_CONFIG BmpConfig,
    IN UINTN X,
    IN UINTN Y
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = Gop->Blt(
        Gop,
        (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)BmpConfig.PixelStart,
        EfiBltBufferToVideo,
        0,0,
        X,Y,
        BmpConfig.Width,BmpConfig.Height,0
    );

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawBmp/Gop->Blt().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: Failed to DrawBmp/Gop->Blt(), you should see the Logo.\n");
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to DrawBmp/Gop->Blt()).\n");
    }else {
        LogWrite("SUCCESS: DrawBmp/Gop->Blt().\n");
    }
    #endif
    return Status;
}
