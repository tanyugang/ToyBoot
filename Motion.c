#include "Motion.h"

EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL Grey = {166, 166, 166, 0};
UINT8 Step = 1;

EFI_STATUS VideoInit(
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
    #ifdef DEBUG
    Print(L"SUCCESS: VideoInit/GetGopHandle().\n");
    #endif
    Status = SetVideoMode(Gop);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to VideoInit/SetVideoMode().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: VideoInit/SetVideoMode().\n");
    #endif
    VideoConfig->FrameBufferBase = Gop->Mode->FrameBufferBase;
    VideoConfig->FrameBufferSize = Gop->Mode->FrameBufferSize;
    VideoConfig->HorizontalResolution = Gop->Mode->Info->HorizontalResolution;
    VideoConfig->VerticalResolution = Gop->Mode->Info->VerticalResolution;
    VideoConfig->PixelsPerScanLine = Gop->Mode->Info->PixelsPerScanLine;
    
    return Status;
}

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
    #ifdef DEBUG
    Print(L"SUCCESS: DrawLogo/GetFileHandle().\n");
    #endif
    EFI_PHYSICAL_ADDRESS LogoAddress;
    Status = ReadFile(Logo, &LogoAddress);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/ReadFile().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: DrawLogo/ReadFile().\n");
    #endif
    BMP_CONFIG BmpConfig;
    Status = BmpTransform(LogoAddress, &BmpConfig);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawLogo/BmpTransform().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: DrawLogo/BmpTransform().\n");
    #endif
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
    #ifdef DEBUG
    Print(L"SUCCESS: DrawLogo/DrawBmp().\n");
    #endif
    return Status;
}

EFI_STATUS DrawStep()
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINTN BlockWidth = Gop->Mode->Info->HorizontalResolution >> 6;
    UINTN BlockHeight = Gop->Mode->Info->VerticalResolution >> 6;
    UINTN StartX = (Gop->Mode->Info->HorizontalResolution - (BlockWidth + GAP) * 10 - GAP) / 2;
    UINTN StartY = (Gop->Mode->Info->VerticalResolution * 3) >> 2;

    UINTN X = StartX + (BlockWidth + GAP) * Step;

    Status = Gop->Blt(Gop, &Grey, EfiBltVideoFill, 0, 0, X, StartY, BlockWidth, BlockHeight, 0);

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to DrawStep().\n", Status);
        #endif
        return Status;
    }
    #ifdef DEBUG
    Print(L"SUCCESS: DrawStep().\n");
    #endif
    Step++;
    return Status;
}
