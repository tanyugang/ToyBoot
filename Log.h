#include <Uefi.h>
#include <Guid/FileInfo.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleFileSystem.h>

#include "Setup.h"

EFI_STATUS LogOpen(EFI_HANDLE ImageHandle);
EFI_STATUS LogWrite(CHAR8 *Message);
EFI_STATUS LogError(EFI_STATUS Code, CHAR8 *ErrorSource);
EFI_STATUS LogClose();