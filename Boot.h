#include  <Guid/Acpi.h>
#include "Motion.h"
#include "Elf.h"

#pragma pack(1)
typedef struct RSDP
{
    CHAR8 Signature[8];
    UINT8 CheckSum;
    CHAR8 OEMID[6];
    UINT8 Revision;
    UINT32 RsdtAddress;
    UINT32 Length;
    UINT64 XsdtAddress;
    UINT8 ExtraCheckSum;
    UINT8 Reserved[3]; 
} RSDP;

typedef struct SDT_HEADER
{
    CHAR8 Signature[4];
    UINT32 Length;
    UINT8 Revision;
    UINT8 CheckSum;
    CHAR8 OEMID[6];
    CHAR8 TableID[8];
    UINT32 OEMRevision;
    UINT32 CreatorID;
    UINT32 CreatorRevision;
} SDT_HEADER;

typedef struct XSDT
{
    SDT_HEADER Header;
    UINT64 PointerOthers[];
} XSDT;

typedef struct MADT
{
    SDT_HEADER Header;
    UINT32 LapicAddress;
    UINT32 Flags;
} MADT;

typedef struct MADT_ENTRY
{
    UINT8 EntryType;
    UINT8 RecordLength;
} MADT_ENTRY;

typedef struct LAPIC
{
    MADT_ENTRY Entry;
    UINT8 ProcessorID;
    UINT8 LapicID;
    UINT32 Flags;
} LAPIC;

typedef struct IOAPIC
{
    MADT_ENTRY Entry;
    UINT8 IopaicID;
    UINT8 Reserved;
    UINT32 IoapicAddress;
    UINT32 GlobalSystemInterruptBase;
} IOAPIC;

typedef struct ISO
{
    MADT_ENTRY Entry;
    UINT8 BusSource;
    UINT8 IrqSourece;
    UINT32 Gsi;
    UINT16 Flags;
} ISO;

typedef struct NMI
{
    MADT_ENTRY Entry;
    UINT8 AcpiProcessorID;
    UINT16 Flags;
    UINT8 Lint;
} NMI;

typedef struct
{
    VOID *Buffer;
    UINTN MapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
} MEMORY_MAP;

typedef struct BOOT_CONFIG
{
    VIDEO_CONFIG VideoConfig;
    MEMORY_MAP   MemoryMap;
    BMP_CONFIG AsciiBmp;
    EFI_PHYSICAL_ADDRESS KernelEntryPoint;
    EFI_PHYSICAL_ADDRESS MadtAddress;
} BOOT_CONFIG; // @Boot.h

typedef struct GDT_SELECTOR
{
    UINT16 LimitLow;
    UINT16 BaseLow;
    UINT8  BaseMiddle;
    UINT8  SysFlag;
    UINT8  LimitHigh;
    UINT8  BaseHigh;
} GDT_SELECTOR;
#pragma pack()

EFI_STATUS GetFontBmp(EFI_HANDLE ImageHandle, CHAR16 *FileName, BMP_CONFIG *BmpConfig);
EFI_STATUS GetMadt(EFI_PHYSICAL_ADDRESS *MadtAddress);
EFI_STATUS JumpToKernel(EFI_HANDLE ImageHandle, BOOT_CONFIG *Bootconfig);