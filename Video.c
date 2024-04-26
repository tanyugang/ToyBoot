#include "Video.h"


EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL Grey = {166, 166, 166, 0};

EFI_STATUS DrawLogo(
    IN EFI_HANDLE ImageHandle
)
{

    EFI_STATUS Status = EFI_SUCCESS;

    CHAR16 *FileName = L"\\Logo.BMP"; 
    UINTN Hor = Gop->Mode->Info->HorizontalResolution;
    UINTN Ver = Gop->Mode->Info->VerticalResolution;

    EFI_FILE_PROTOCOL *Logo;
    Status = GetFileHandle(ImageHandle, FileName, &Logo);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/GetFileHandle().\n", Status);
        #endif
        return Status;
    }

    EFI_PHYSICAL_ADDRESS LogoAddress;
    Status = ReadFile(Logo, &LogoAddress);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/ReadFile().\n", Status);
        #endif
        return Status;
    }

    BMP_CONFIG BmpConfig;
    Status = BmpTransform(LogoAddress, &BmpConfig);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/BmpTransform().\n", Status);
        #endif
        return Status;
    }

    UINTN X = (Hor - BmpConfig.Width) / 2;
    UINTN Y = (Ver - BmpConfig.Height) / 2;

    Status = DrawBmp(Gop, BmpConfig, X, Y);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/DrawBmp().\n", Status);
        #endif
        return Status;
    }

    return Status;
}

EFI_STATUS GetVideoConfig(
    IN EFI_HANDLE ImageHandle,
    OUT VIDEO_CONFIG *VideoConfig
)
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = GetGopHandle(ImageHandle, &Gop);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to VideoInit/GetGopHandle().\n", Status);
        #endif
        return Status;
    }

    Status = SetVideoMode(Gop);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to VideoInit/SetVideoMode().\n", Status);
        #endif
        return Status;
    }

    VideoConfig->FrameBufferBase = Gop->Mode->FrameBufferBase;
    VideoConfig->FrameBufferSize = Gop->Mode->FrameBufferSize;
    VideoConfig->HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
    VideoConfig->VerticalResolution = Gop->Mode->Info->VerticalResolution;
    VideoConfig->PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
    
    return Status;
}

EFI_STATUS GetGopHandle(
    IN EFI_HANDLE ImageHandle,
    OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;

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

    return Status;
}

EFI_STATUS SetVideoMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
)
{
    EFI_STATUS Status = EFI_SUCCESS;

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

    Status = Gop->SetMode(Gop, ModeIndex);  

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to SetVideoMode/Gop->SetMode().\n", Status);
        #endif
        return Status;
    }

    return Status;
}


EFI_STATUS BmpTransform(
    IN EFI_PHYSICAL_ADDRESS BmpBase,
    OUT BMP_CONFIG *BmpConfig
)    
{  
    EFI_STATUS Status = EFI_SUCCESS;

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

    return Status;
}
