#include "Motion.h"
#include "Elf.h"

#pragma pack(1)

typedef struct
{
    VIDEO_CONFIG VideoConfig;
    BMP_CONFIG AsciiBmp;
} BOOT_CONFIG; // @Boot.h

#pragma pack()