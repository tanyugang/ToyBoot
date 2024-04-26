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
    
    if(EFI_ERROR(Status))
    {   
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFileHandle/gBS->LocateHanleBuffer() of SimpleFileSystemProtocol.\n", Status);
        #endif
        return Status;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = gBS->OpenProtocol(
        HandleBuffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: Failed to GetFileHandle/gBS->OpenProtocol() GraphicsOutputProtocol.\n");
        #endif
        return Status;
    }

    EFI_FILE_PROTOCOL *Root;
    Status = FileSystem->OpenVolume(
        FileSystem,
        &Root
    );
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFileHandle/FileSystem->OpenVolume().\n", Status);
        #endif
        return Status;
    }
    
    Status = Root->Open(
        Root,
        FileHandle,
        FileName, 
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to GetFileHandle/Root->Open().\n", Status);
        #endif
        return Status;
    }
      
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

   
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to ReadFile/gBS->AllocatePool().\n", Status);
        #endif
        return Status;
    }
       

    Status = File->GetInfo(
        File,
        &gEfiFileInfoGuid,
        &InfoSize,
        FileInfo
    );
    
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to ReadFile/File->GetInfo().\n", Status);
        #endif
        return Status;
    }

    UINTN FilePageSize = (FileInfo->FileSize >> 12) + 1;
    
    EFI_PHYSICAL_ADDRESS FileBufferAddress;
    Status = gBS->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        FilePageSize,
        &FileBufferAddress
    );

    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to ReadFile/gBS->AllocatePages().\n", Status);
        #endif
        return Status;
    }

    UINTN ReadSize = FileInfo->FileSize;
    Status = File->Read(
        File,
        &ReadSize,
        (VOID *)FileBufferAddress
    );
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to ReadFile/File->Read().\n", Status);
        #endif
        return Status;
    }
   

    Status = gBS->FreePool(FileInfo);
    
    if(EFI_ERROR(Status))
    {
        #ifdef DEBUG
        Print(L"ERROR: %r. Failed to ReadFile/gBS->FreePool().\n", Status);
        #endif
        return Status;
    }

    *FileBase = FileBufferAddress;
    return Status;
}
