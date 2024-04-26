#include <Uefi.h>

//#define DEBUG

UINTN GetValue(
    IN EFI_PHYSICAL_ADDRESS StartAddress,
    IN UINTN Offset,
    IN UINTN Size
);
