#include "Log.h"

EFI_FILE_PROTOCOL *LogFile;

EFI_STATUS LogOpen(EFI_HANDLE ImageHandle)
{
    EFI_STATUS Status = EFI_SUCCESS;
    
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    UINTN HandleCount = 0;
    EFI_HANDLE *Buffer = NULL;
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &HandleCount,
        &Buffer
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogOpen/gBS->LocateHanleBuffer() of SimpleFileSystemProtocol.\n", Status);
        return Status;
    }
    Print(L"SUCCESS: LogOpen/gBS->LocateHanleBuffer(). Get %d handles that supported SimpleFileSystemProtocol.\n", HandleCount);
    #endif
    
    Status = gBS->OpenProtocol(
        Buffer[0],
        &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&FileSystem,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogOpen/gBS->OpenProtocol() SimpleFileSystemProtocol.\n", Status);
        return Status;
    }
    Print(L"SUCCESS: LogOpen/gBS->OpenProtocol().\n");
    #endif
    EFI_FILE_PROTOCOL *File = NULL;
    Status = FileSystem->OpenVolume(
        FileSystem,
        &File
    );
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogOpen/FileSystem->OpenVolume().\n", Status);
        return Status;
    }

    Print(L"SUCCESS: LogOpen/FileSystem->OpenVolume().\n");
    #endif

    File->Open(
        File,
        &LogFile,
        L"\\Log.txt",
        EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
        EFI_FILE_ARCHIVE
    );

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogOpen/File->Open() Log.txt\n", Status);
        return Status;
    }

    Print(L"SUCCESS: LogOpen/File->Open().\n");
    #endif
    
    Status = LogWrite("LogFile is ready.\n");
  
    return Status;
}

EFI_STATUS LogWrite(CHAR8 *Message)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN MsgLen = AsciiStrLen(Message);
    Status = LogFile->Write(LogFile, &MsgLen, Message);

    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogWrite/LogFile->Write() LogFile.\n", Status);
        return Status;
    }
    //Print(L"SUCCESS: LogWrite/LogFile->Write(). \n");
    #endif
    Status = LogFile->Flush(LogFile);
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogWrite/LogFile->Flush() LogFile.\n", Status);
        return Status;
    }
    //Print(L"SUCCESS: LogWrite/LogFile->Flush().\n");
    #endif
    return Status;
}

EFI_STATUS LogClose()
{
    EFI_STATUS Status = EFI_SUCCESS;
    Status = LogFile->Close(LogFile);
    #ifdef DEBUG
    if(EFI_ERROR(Status))
    {
        Print(L"ERROR: %r. Failed to LogClose/LogFile->Close() LogFile.\n", Status);
        return Status;
    }
    Print(L"SUCCESS: LogClose/LogFile->Close().\n");
    #endif
    return Status;
}


EFI_STATUS LogError(EFI_STATUS Code, CHAR8 *ErrorSource)
{
    EFI_STATUS Status = EFI_SUCCESS;
    switch (Code)
    {
        case EFI_SUCCESS:
            Status = LogWrite("SUCCESS:EFI_SUCCESS. ");
            break;
        case EFI_LOAD_ERROR:  
            Status = LogWrite("ERROR:  EFI_LOAD_ERROR. ");
            break;
        case EFI_INVALID_PARAMETER:
            Status = LogWrite("ERROR:  EFI_INVALID_PARAMETER. ");
            break;
        case EFI_UNSUPPORTED:
            Status = LogWrite("ERROR:  EFI_UNSUPPORTED. ");
            break;
        case EFI_BAD_BUFFER_SIZE:
            Status = LogWrite("ERROR:  EFI_BAD_BUFFER_SIZE. ");
            break;
        case EFI_BUFFER_TOO_SMALL:
            Status = LogWrite("ERROR:  EFI_BUFFER_TOO_SMALL. ");
            break;
        case EFI_NOT_READY:
            Status = LogWrite("ERROR:  EFI_NOT_READY. ");
            break;
        case EFI_DEVICE_ERROR:  
            Status = LogWrite("ERROR:  EFI_DEVICE_ERROR. ");
            break;
        case EFI_WRITE_PROTECTED:
            Status = LogWrite("ERROR:  EFI_WRITE_PROTECTED. ");
            break;
        case EFI_OUT_OF_RESOURCES:
            Status = LogWrite("ERROR:  EFI_OUT_OF_RESOURCES. ");
            break;
        case EFI_VOLUME_CORRUPTED:
            Status = LogWrite("ERROR:  EFI_VOLUME_CORRUPTED. ");
            break;
        case EFI_VOLUME_FULL:
            Status = LogWrite("ERROR:  EFI_VOLUME_FULL. ");
            break;
        case EFI_NO_MEDIA:
            Status = LogWrite("ERROR:  EFI_NO_MEDIA. ");
            break;
        case EFI_MEDIA_CHANGED:
            Status = LogWrite("ERROR:  EFI_MEDIA_CHANGED. ");
            break;
        case EFI_NOT_FOUND:
            Status = LogWrite("ERROR:  EFI_NOT_FOUND. ");
            break;
        case EFI_ACCESS_DENIED:
            Status = LogWrite("ERROR:  EFI_ACCESS_DENIED. ");
            break;
        case EFI_NO_RESPONSE:
            Status = LogWrite("ERROR:  EFI_NO_RESPONSE. ");
            break;
        case EFI_NO_MAPPING:
            Status = LogWrite("ERROR:  EFI_NO_MAPPING. ");
            break;
        case EFI_TIMEOUT:
            Status = LogWrite("ERROR:  EFI_TIMEOUT. ");
            break;
        case EFI_NOT_STARTED:
            Status = LogWrite("ERROR:  EFI_NOT_STARTED. ");
            break;
        case EFI_ALREADY_STARTED:
            Status = LogWrite("ERROR:  EFI_ALREADY_STARTED. ");
            break;
        case EFI_ABORTED:
            Status = LogWrite("ERROR:  EFI_ABORTED. ");
            break;
        case EFI_ICMP_ERROR:  
            Status = LogWrite("ERROR:  EFI_ICMP_ERROR");
            break;
        case EFI_TFTP_ERROR:  
            Status = LogWrite("ERROR:  EFI_TFTP_ERROR. ");
            break;
        case EFI_PROTOCOL_ERROR:  
            Status = LogWrite("ERROR:  EFI_PROTOCOL_ERROR. ");
            break;
        case EFI_INCOMPATIBLE_VERSION:
            Status = LogWrite("ERROR:  EFI_INCOMPATIBLE_VERSION. ");
            break;
        case EFI_SECURITY_VIOLATION:
            Status = LogWrite("ERROR:  EFI_SECURITY_VIOLATION. ");
            break;
        case EFI_CRC_ERROR:  
            Status = LogWrite("ERROR:  EFI_CRC_ERROR. ");
            break;
        case EFI_END_OF_MEDIA:
            Status = LogWrite("ERROR:  EFI_END_OF_MEDIA. ");
            break;
        case EFI_END_OF_FILE:
            Status = LogWrite("ERROR:  EFI_END_OF_FILE. ");
            break;
        case EFI_INVALID_LANGUAGE:
            Status = LogWrite("ERROR:  EFI_INVALID_LANGUAGE. ");
            break;
        case EFI_COMPROMISED_DATA:
            Status = LogWrite("ERROR:  EFI_COMPROMISED_DATA. ");
            break;
        case EFI_IP_ADDRESS_CONFLICT:
            Status = LogWrite("ERROR:  EFI_IP_ADDRESS_CONFLICT. ");
            break;
        case EFI_HTTP_ERROR:  
            Status = LogWrite("ERROR:  EFI_HTTP_ERROR. ");
            break;
        case EFI_WARN_DELETE_FAILURE:
            Status = LogWrite("ERROR:  EFI_WARN_DELETE_FAILURE. ");
            break;
        case EFI_WARN_UNKNOWN_GLYPH:
            Status = LogWrite("ERROR:  EFI_WARN_UNKNOWN_GLYPH. ");
            break;
        case EFI_WARN_WRITE_FAILURE:
            Status = LogWrite("ERROR:  EFI_WARN_WRITE_FAILURE. ");
            break;
        case EFI_WARN_BUFFER_TOO_SMALL:
            Status = LogWrite("ERROR:  EFI_WARN_BUFFER_TOO_SMALL. ");
            break;
        case EFI_WARN_STALE_DATA:
            Status = LogWrite("ERROR:  EFI_WARN_STALE_DATA. ");
            break;
        case EFI_WARN_FILE_SYSTEM:
            Status = LogWrite("ERROR:  EFI_WARN_FILE_SYSTEM. ");
            break;
        case EFI_WARN_RESET_REQUIRED:
            Status = LogWrite("ERROR:  EFI_WARN_RESET_REQUIRED. ");
            break;
        default:
            Status = LogWrite("ERROR:  Something must be wrong. ");
            break;
    }
    Status = LogWrite(ErrorSource);
    //Status = LogWrite("\n\0");
    return Status;
}
