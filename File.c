#include "File.h"

EFI_STATUS GetFileHandle(
    IN EFI_HANDLE ImageHandle,
    IN CHAR16 *FileName,
    OUT EFI_FILE_PROTOCOL **FileHandle
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN HandleCount = 0;
    EFI_HANDLE *HandleBuffer;
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to GetFileHandle/gBS->LocateHanleBuffer() of SimpleFileSystemProtocol.\n", Status);
        return Status;
    }
    Print(L"SUCCESS: GetFileHandle/gBS->LocateHanleBuffer(). Get %d handles that supported SimpleFileSystemProtocol.\n", HandleCount);
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "GetFileHandle/gBS->LocateHanleBuffer().\n");
    }else {
        LogWrite("SUCCESS: GetFileHandle/gBS->LocateHanleBuffer().\n");
    }
    #endif
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = gBS->OpenProtocol(
        HandleBuffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: Failed to GetFileHandle/gBS->OpenProtocol() GraphicsOutputProtocol.\n");
        return Status;
    }
    Print(L"SUCCESS: GetFileHandle/gBS->OpenProtocol().\n"); 
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "GetFileHandle/gBS->OpenProtocol() GraphicsOutputProtocol.\n");
    }else {
        LogWrite("SUCCESS: GetFileHandle/gBS->OpenProtocol().\n");
    }
    #endif
    EFI_FILE_PROTOCOL *Root;
    Status = FileSystem->OpenVolume(
        FileSystem,
        &Root
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to GetFileHandle/FileSystem->OpenVolume().\n", Status);
        return Status;
    }
    Print(L"SUCCESS: GetFileHandle/FileSystem->OpenVolume().\n");
    #endif
    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetFileHandle/FileSystem->OpenVolume().\n");
    }else {
        LogWrite("SUCCESS: GetFileHandle/FileSystem->OpenVolume().\n");
    }
    #endif
    Status = Root->Open(
        Root,
        FileHandle,
        FileName, 
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );


    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to GetFileHandle/Root->Open().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: GetFileHandle/Root->Open().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to GetFileHandle/Root->Open().\n");
    }else {
        LogWrite("SUCCESS: GetFileHandle/Root->Open().\n");
    }
    #endif
    return Status;
} 
 
EFI_STATUS ReadFile(
    IN EFI_FILE_PROTOCOL *File,
    OUT EFI_PHYSICAL_ADDRESS *FileBase
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_INFO *FileInfo;

    UINTN InfoSize = sizeof(EFI_FILE_INFO) + 128;
    Status = gBS->AllocatePool(
        EfiLoaderData,
        InfoSize,
        (VOID **)&FileInfo
    );

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to ReadFile/gBS->AllocatePool().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: ReadFile/gBS->AllocatePool().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to ReadFile/gBS->AllocatePool().\n");
    }else {
        LogWrite("SUCCESS: ReadFile/gBS->AllocatePool().\n");
    }
    #endif
    return Status;

    Status = File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );
    
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to ReadFile/File->GetInfo().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: ReadFile/File->GetInfo().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to ReadFile/File->GetInfo().\n");
    }else {
        LogWrite("SUCCESS: ReadFile/File->GetInfo().\n");
    }
    #endif
    return Status;
    
    UINTN FilePageSize = (FileInfo->FileSize >> 12) + 1;
    
    EFI_PHYSICAL_ADDRESS FileBufferAddress;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        FilePageSize,
        &FileBufferAddress
    );

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to ReadFile/gBS->AllocatePages().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: ReadFile/gBS->AllocatePages().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to ReadFile/gBS->AllocatePages().\n");
    }else {
        LogWrite("SUCCESS: ReadFile/gBS->AllocatePages().\n");
    }
    #endif

    UINTN ReadSize = FileInfo->FileSize;
    Status = File->Read(
        File,
        &ReadSize,
        (VOID *)FileBufferAddress
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to ReadFile/File->Read().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: ReadFile/File->Read().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to ReadFile/File->Read().\n");
    }else {
        LogWrite("SUCCESS: ReadFile/File->Read().\n");
    }
    #endif
    Status = gBS->FreePool(FileInfo);
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to ReadFilegBS->FreePool().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: ReadFile/gBS->FreePool().\n");
    #endif

    #ifdef LOG
    if(EFI_ERROR(Status))
    {
        LogError(Status, "Failed to ReadFile/gBS->FreePool().\n");
    }else {
        LogWrite("SUCCESS: ReadFile/gBS->FreePool().\n");
    }
    #endif
    *FileBase = FileBufferAddress;
    return Status;
}
